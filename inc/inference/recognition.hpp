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

#define BODY_KP_NUM     17

namespace recognition {
    const int BodySkeletonIndex[BODY_KP_NUM][2] = {
        {1, 3},
        {1, 0},
        {2, 4},
        {2, 0},
        {0, 5},
        {0, 6},
        {5, 7},
        {7, 9},
        {6, 8},
        {8, 10},
        {5, 11},
        {6, 12},
        {11, 12},
        {11, 13},
        {13, 15},
        {12, 14},
        {14, 16}
    };

}