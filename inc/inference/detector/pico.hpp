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
#include "inference/detection.hpp"

#include "utils/io.hpp"
#include "utils/frame_utils.hpp"

#include <algorithm>

namespace infer
{
    struct PicoConfig
    {
        std::vector<int> strides;
        int num_class;
        float cls_thresh;
        float nms_thresh;
        std::vector<float> zps;
        std::vector<float> scales;
        std::vector<int> want_classes;
    };

    class Pico : public EngineWrapper
    {
    public:
        Pico()
        { }

        ~Pico() = default;

        void SetConfig(const PicoConfig& config)
        {
            m_config = config;
        }

        PicoConfig GetConfig() const
        {
            return m_config;
        }

        int Detect(const AX_VIDEO_FRAME_T& img, 
                std::vector<detection::Object>& outputs)
        {
            if (!m_hasInit)
                return -1;

            int ret = 0;

            AX_VIDEO_FRAME_T dst;
            ret = Preprocess(img, dst);
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

            // generate proposals
            std::vector<detection::Object> proposals;
            for (int i = 0; i < m_output_num; i++)
            {
                auto& buf = m_io.pOutputs[i];
                axALGO::cache_io_flush(&buf);

                AX_U8* puBuf = (AX_U8*)buf.pVirAddr;
                detection::generate_pico_proposals(puBuf, m_config.strides[i], m_input_size[0], m_input_size[1], m_config.cls_thresh, proposals, m_config.num_class, m_config.scales[i], m_config.zps[i]);
            }

            // nms & rescale coords & select class
            outputs.clear();
            detection::get_out_bbox(proposals, outputs, m_config.nms_thresh, m_input_size[0], m_input_size[1], img.u32Height, img.u32Width);

            if (!m_config.want_classes.empty())
            {
                for (auto it = outputs.begin(); it != outputs.end(); )
                {
                    if (std::find(m_config.want_classes.begin(), m_config.want_classes.end(), it->label) == m_config.want_classes.end())
                        it = outputs.erase(it);
                    else
                        it++;
                }
            }

            return 0;
        }

    protected:
        PicoConfig m_config;
    };
}