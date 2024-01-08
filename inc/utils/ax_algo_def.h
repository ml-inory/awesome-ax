/**************************************************************************************************
 *
 * Copyright (c) 2019-2023 Axera Semiconductor (Ningbo) Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor (Ningbo) Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor (Ningbo) Co., Ltd.
 *
 **************************************************************************************************/

#ifndef __AX_ALGO_DEF_H__
#define __AX_ALGO_DEF_H__

#include "ax_algo_err.h"
#include "ax_algo_log.h"

#define CHECK_PTR(p)                     \
    do {                                 \
        if (!p) {                        \
            ALOGE("%s nil pointer\n", #p);      \
            return AX_ERR_ALGO_NULL_PTR; \
        }                                \
    } while (0)

#endif // __AX_ALGO_DEF_H__