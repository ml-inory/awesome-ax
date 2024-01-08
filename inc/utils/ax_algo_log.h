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
#include "ax_global_type.h"
#include "ax_sys_log.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ALGO_LOG_TAG "ALGO"

#define ALOGE(fmt, ...) AX_LOG_ERR(ALGO_LOG_TAG, AX_ID_SKEL, fmt, ##__VA_ARGS__)
#define ALOGW(fmt, ...) AX_LOG_WARN(ALGO_LOG_TAG, AX_ID_SKEL, fmt, ##__VA_ARGS__)
#define ALOGI(fmt, ...) AX_LOG_INFO(ALGO_LOG_TAG, AX_ID_SKEL, fmt, ##__VA_ARGS__)
#define ALOGD(fmt, ...) AX_LOG_DBG(ALGO_LOG_TAG, AX_ID_SKEL, fmt, ##__VA_ARGS__)
#define ALOGN(fmt, ...) AX_LOG_NOTICE(ALGO_LOG_TAG, AX_ID_SKEL, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
