/**************************************************************************************************
 *
 * Copyright (c) 2019-2023 Axera Semiconductor (Shanghai) Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor (Shanghai) Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor (Shanghai) Co., Ltd.
 *
 **************************************************************************************************/

#pragma once

#include "inference/engine_wrapper.hpp"
#include "inference/recognition.hpp"

#include "utils/io.hpp"
#include "utils/frame_utils.hpp"

#include "opencv2/core.hpp"
// #include "opencv2/imgproc.hpp"

#include <algorithm>

namespace infer
{
    const std::vector<cv::Scalar> COCO_SKELETON_Colors = {
        {255,   0,      0},
        {255,   85,     0},
        {255,   170,    0},
        {255,   255,    0},
        {170,   255,    0},
        {85,    255,    0},
        {0,     255,    0},
        {0,     255,    85},
        {0,     255,    170},
        {0,     255,    255},
        {0,     170,    255},
        {0,     85,     255},
        {0,     0,      255},
        {85,    0,      255},
        {170,   0,      255},
        {255,   0,      255},
        {255,   0,      170},
        {255,   0,      85}
    };

    void find_max_2d(float* buf, int width, int height, int* max_idx_width, int* max_idx_height, float* max_value) 
    {
        *max_value = -10.f;
        *max_idx_width = 0;
        *max_idx_height = 0;
        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w++) {
                float score = buf[h * width + w];
                if (score > *max_value) {
                    *max_value = score;
                    *max_idx_height = h;
                    *max_idx_width = w;
                }
            }
        }
    }

    class PoseEstimator : public EngineWrapper
    {
    public:
        PoseEstimator()
        { }

        ~PoseEstimator() = default;

        int Postprocess(const AX_ENGINE_IOMETA_T& output_info, 
                        AX_ENGINE_IO_BUFFER_T& buf, 
                        const cv::Rect& bbox,
                        std::vector<cv::Point>& outputs)
        {
            int num_kp = output_info.pShape[1];
            int feat_h = output_info.pShape[2];
            int feat_w = output_info.pShape[3];
            int c_stride = feat_h * feat_w;

            float max_val;
            int x, y;

            float aspect_w = bbox.width  * 1.0f / m_input_size[1];
            float aspect_h = bbox.height * 1.0f / m_input_size[0];

            int scale_w = m_input_size[1] / feat_w;
            int scale_h = m_input_size[0] / feat_h;

            float* feat_ptr = (float*)buf.pVirAddr;

            outputs.clear();
            for (int i = 0; i < num_kp; i++)
            {
                find_max_2d(feat_ptr + i * c_stride, feat_w, feat_h, &x, &y, &max_val);
                outputs.emplace_back(x * scale_w * aspect_w + bbox.x, y * scale_h * aspect_h + bbox.y);
            }

            return 0;
        }                        

        int Recognize(const AX_VIDEO_FRAME_T& img, 
                const cv::Rect& bbox,
                std::vector<cv::Point>& outputs)
        {
            if (!m_hasInit)
                return -1;

            int ret = 0;

            AX_VIDEO_FRAME_T dst;
            ret = Preprocess(img, dst, bbox);
            if (ret != 0)
            {
                utils::FreeFrame(dst);
                return ret;
            }

            ret = Run(dst);
            if (ret != 0)
            {
                utils::FreeFrame(dst);
                return ret;
            }
            utils::FreeFrame(dst);

            axALGO::cache_io_flush(&m_io.pOutputs[0]);

            ret = Postprocess(m_io_info->pOutputs[0], m_io.pOutputs[0], bbox, outputs);
            if (ret != 0)
            {
                return ret;
            }

            return 0;
        }

        // void Draw(cv::Mat& img, const cv::Rect& roi, const std::vector<cv::Point>& pose_outputs)
        // {
        //     for (int i = 0; i < BODY_KP_NUM; i++)
        //     {
        //         int kpt_a = recognition::BodySkeletonIndex[i][0];
        //         int kpt_b = recognition::BodySkeletonIndex[i][1];
        //         cv::Point kp_a = roi.tl() + pose_outputs[kpt_a];
        //         cv::Point kp_b = roi.tl() + pose_outputs[kpt_b];
        //         cv::circle(img, kp_a, 6, COCO_SKELETON_Colors[i], -1);
        //         cv::circle(img, kp_b, 6, COCO_SKELETON_Colors[i], -1);
        //         cv::line(img, kp_a, kp_b, COCO_SKELETON_Colors[i], 2);
        //     }
        // }
    };
}