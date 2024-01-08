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

#if 0

#include "inference/detector/yolox.hpp"

namespace infer
{
    class BodyDetector : public YoloX
    {
    public:
        BodyDetector()
        {
            m_config.obj_thresh = 0.01f;
            m_config.cls_thresh = 0.5f;
            m_config.nms_thresh = 0.45f;
            m_config.min_size = cv::Size(1, 1);
            m_config.strides = std::vector<std::vector<int>>{{8}, {16}, {32}};
            m_config.want_classes = std::vector<int>{0};
            m_config.zps = std::vector<float>{119, 112, 117};
            m_config.scales = std::vector<float>{0.08084503561258316, 0.07241208851337433, 0.0644543245434761};
        }

        ~BodyDetector() = default;
    };
}

#endif