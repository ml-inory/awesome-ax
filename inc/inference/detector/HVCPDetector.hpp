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

#include "inference/detector/pico.hpp"

namespace infer
{
    class HVCPDetector : public Pico
    {
    public:
        HVCPDetector()
        {
            m_config.num_class = 2;
            m_config.cls_thresh = 0.5f;
            m_config.nms_thresh = 0.6f;
            m_config.want_classes = std::vector<int>{0};
            m_config.strides = std::vector<int>{8, 16, 32};
            m_config.zps = std::vector<float>{119, 112, 117};
            m_config.scales = std::vector<float>{0.08084503561258316, 0.07241208851337433, 0.0644543245434761};
        }

        ~HVCPDetector() = default;
    };
}

