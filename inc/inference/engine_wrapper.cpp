/**************************************************************************************************
 *
 * Copyright (c) 2019-2023 Axera Semiconductor (Shanghai) Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor (Shanghai) Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor (Shanghai) Co., Ltd.
 *
 **************************************************************************************************/
#include "inference/engine_wrapper.hpp"
#include "inference/engine_env.hpp"

#include "utils/ax_algo_log.h"
#include "utils/io.hpp"
#include "utils/frame_utils.hpp"

#include "err.hpp"

#include <stdlib.h>

/// @brief npu type
typedef enum axALGO_NPU_TYPE_E {
    AX_ALGO_NPU_DEFAULT = 0,        /* running under default NPU according to system */
    AX_ALGO_STD_VNPU_1 = (1 << 0),  /* running under STD VNPU1 */
    AX_ALGO_STD_VNPU_2 = (1 << 1),  /* running under STD VNPU2 */
} AX_ALGO_NPU_TYPE_E;


#if defined(CHIP_AX650)
static const char *strAlgoModelType[AX_ENGINE_MODEL_TYPE_BUTT] = {"3.6T", "7.2T", "10.8T"};
#endif

#if defined(CHIP_AX620E)
static const char *strAlgoModelType[AX_ENGINE_MODEL_TYPE_BUTT] = {"HalfOCM", "FullOCM"};
#endif


#if defined(CHIP_AX650)
static AX_S32 CheckModelVNpu(const std::string &strModel, const AX_ENGINE_MODEL_TYPE_T &eModelType, const AX_S32 &nNpuType, AX_U32 &nNpuSet) {
    AX_ENGINE_NPU_ATTR_T stNpuAttr;
    memset(&stNpuAttr, 0x00, sizeof(stNpuAttr));

    auto ret = AX_ENGINE_GetVNPUAttr(&stNpuAttr);
    if (ret == 0) {
        // VNPU DISABLE
        if (stNpuAttr.eHardMode == AX_ENGINE_VIRTUAL_NPU_DISABLE) {
            nNpuSet = 0x01; // NON-VNPU (0b111)
            ALOGN("%s will run under VNPU-DISABLE [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
        }
        // STD VNPU
        else if (stNpuAttr.eHardMode == AX_ENGINE_VIRTUAL_NPU_STD) {
            // 7.2T & 10.8T no allow
            if (eModelType == AX_ENGINE_MODEL_TYPE1
                || eModelType == AX_ENGINE_MODEL_TYPE2) {
                printf("%s model type%d: [%s], no allow run under STD VNPU", strModel.c_str(), eModelType, strAlgoModelType[eModelType]);
                return AX_ERR_ALGO_ILLEGAL_PARAM;
            }

            // default STD VNPU2
            if (nNpuType == 0) {
                nNpuSet = 0x02; // VNPU2 (0b010)
                ALOGN("%s will run under default STD-VNPU2 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
            }
            else {
                if (nNpuType & AX_ALGO_STD_VNPU_1) {
                    nNpuSet |= 0x01; // VNPU1 (0b001)
                    ALOGN("%s will run under STD-VNPU1 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
                }
                if (nNpuType & AX_ALGO_STD_VNPU_2) {
                    nNpuSet |= 0x02; // VNPU2 (0b010)
                    ALOGN("%s will run under STD-VNPU2 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
                }
                if (nNpuType & AX_ALGO_STD_VNPU_3) {
                    nNpuSet |= 0x04; // VNPU3 (0b100)
                    ALOGN("%s will run under STD-VNPU3 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
                }
            }
        }
        // BL VNPU
        else if (stNpuAttr.eHardMode == AX_ENGINE_VIRTUAL_NPU_BIG_LITTLE) {
            // 10.8T no allow
            if (eModelType == AX_ENGINE_MODEL_TYPE2) {
                printf("%s model type%d: [%s], no allow run under BL VNPU", strModel.c_str(), eModelType, strAlgoModelType[eModelType]);
                return AX_ERR_ALGO_ILLEGAL_PARAM;
            }

            // default BL VNPU
            if (nNpuType == 0) {
                // 7.2T default BL VNPU1
                if (eModelType == AX_ENGINE_MODEL_TYPE1) {
                    nNpuSet = 0x01; // VNPU1 (0b001)
                    ALOGN("%s will run under default BL-VNPU1 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
                }
                // 3.6T default BL VNPU2
                else {
                    nNpuSet = 0x02; // VNPU2 (0b010)
                    ALOGN("%s will run under default BL-VNPU2 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
                }
            }
            else {
                // 7.2T
                if (eModelType == AX_ENGINE_MODEL_TYPE1) {
                    // no allow set to BL VNPU2
                    if (nNpuType & AX_SKEL_BL_VNPU_2) {
                        printf("%s model type%d: [%s], no allow run under BL VNPU2", strModel.c_str(), eModelType, strAlgoModelType[eModelType]);
                        return AX_ERR_ALGO_ILLEGAL_PARAM;
                    }
                    if (nNpuType & AX_SKEL_BL_VNPU_1) {
                        nNpuSet |= 0x01; // VNPU1 (0b001)
                        ALOGN("%s will run under BL-VNPU1 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
                    }
                }
                // 3.6T
                else {
                    if (nNpuType & AX_SKEL_BL_VNPU_1) {
                        nNpuSet |= 0x01; // VNPU1 (0b001)
                        ALOGN("%s will run under BL-VNPU1 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
                    }
                    if (nNpuType & AX_SKEL_BL_VNPU_2) {
                        nNpuSet |= 0x02; // VNPU2 (0b010)
                        ALOGN("%s will run under BL-VNPU2 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
                    }
                }
            }
        }
    }
    else {
        printf("AX_ENGINE_GetVNPUAttr fail ret = %x", ret);
    }

    return ret;
}
#endif

#if defined(CHIP_AX620E)
static AX_S32 CheckModelVNpu(const std::string &strModel, const AX_ENGINE_MODEL_TYPE_T &eModelType, const AX_S32 &nNpuType, AX_U32 &nNpuSet) {
    AX_ENGINE_NPU_ATTR_T stNpuAttr;
    memset(&stNpuAttr, 0x00, sizeof(stNpuAttr));

    auto ret = AX_ENGINE_GetVNPUAttr(&stNpuAttr);
    if (ret == 0) {
        // VNPU DISABLE
        if (stNpuAttr.eHardMode == AX_ENGINE_VIRTUAL_NPU_DISABLE) {
            nNpuSet = 0x01; // NON-VNPU (0b111)
            ALOGN("%s will run under VNPU-DISABLE [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
        }
        // STD VNPU
        else if (stNpuAttr.eHardMode == AX_ENGINE_VIRTUAL_NPU_ENABLE) {
            // full ocm model was no allowned
            if (eModelType == AX_ENGINE_MODEL_TYPE1) {
                printf("%s model type%d: [%s], no allow run under STD VNPU", strModel.c_str(), eModelType, strAlgoModelType[eModelType]);
                return AX_ERR_ALGO_ILLEGAL_PARAM;
            }

            // default STD VNPU2
            if (nNpuType == 0) {
                nNpuSet = 0x02; // VNPU2 (0b010)
                ALOGN("%s will run under default STD-VNPU2 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
            }
            else {
                if (nNpuType & AX_ALGO_STD_VNPU_1) {
                    nNpuSet |= 0x01; // VNPU1 (0b001)
                    ALOGN("%s will run under STD-VNPU1 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
                }
                if (nNpuType & AX_ALGO_STD_VNPU_2) {
                    nNpuSet |= 0x02; // VNPU2 (0b010)
                    ALOGN("%s will run under STD-VNPU2 [%s]", strModel.c_str(), strAlgoModelType[eModelType]);
                }
            }
        }
    }
    else {
        printf("AX_ENGINE_GetVNPUAttr fail ret = %x", ret);
    }

    return ret;
}
#endif

namespace infer
{
    int EngineWrapper::Init(const std::string& strModelPath)
    {
        AX_S32 ret = 0;

        // 1. load model
        AX_BOOL bLoadModelUseCmm = AX_FALSE;
        AX_CHAR *pModelBufferVirAddr = nullptr;
        AX_U64 u64ModelBufferPhyAddr = 0;
        AX_U32 nModelBufferSize = 0;

        const char *strLoadModelUseCmmStr = getenv(ALGO_LOAD_MODEL_USE_CMM_ENV_STR);
        if (strLoadModelUseCmmStr) {
            bLoadModelUseCmm = (AX_BOOL)atoi(strLoadModelUseCmmStr);
        }

        ret = AX_SYS_Init();
        if (ret != 0)
        {
            printf("AX_SYS_Init failed! ret={0x%8x}\n", ret);
            return AX_ERR_ALGO_ILLEGAL_PARAM;
        }

    #if defined(CHIP_AX650)
        AX_ENGINE_NPU_ATTR_T attr;
        memset(&attr, 0, sizeof(AX_ENGINE_NPU_ATTR_T));
        attr.eHardMode = AX_ENGINE_VIRTUAL_NPU_DISABLE;
        ret = AX_ENGINE_Init(&attr);
    #endif
    #if defined(CHIP_AX620E)
        ret = AX_ENGINE_Init();
    #endif
        if (ret != 0)
        {
            printf("AXEngine init failed! ret={0x%8x}\n", ret);
            return AX_ERR_ALGO_ILLEGAL_PARAM;
        }

        std::vector<char> model_buffer;

        if (bLoadModelUseCmm) {
            if (!axALGO::read_file(strModelPath, (AX_VOID **)&pModelBufferVirAddr, u64ModelBufferPhyAddr, nModelBufferSize)) {
                printf("ALGO read model(%s) fail\n", strModelPath.c_str());
                return AX_ERR_ALGO_ILLEGAL_PARAM;
            }
        }
        else {
            if (!axALGO::read_file(strModelPath, model_buffer)) {
                printf("ALGO read model(%s) fail\n", strModelPath.c_str());
                return AX_ERR_ALGO_ILLEGAL_PARAM;
            }

            pModelBufferVirAddr = model_buffer.data();
            nModelBufferSize = model_buffer.size();
        }

        auto freeModelBuffer = [&]() {
            if (bLoadModelUseCmm) {
                if (u64ModelBufferPhyAddr != 0) {
                    AX_SYS_MemFree(u64ModelBufferPhyAddr, &pModelBufferVirAddr);
                }
            }
            else {
                std::vector<char>().swap(model_buffer);
            }
            return;
        };

        // 1.1 Get Model Type
        // AX_ENGINE_MODEL_TYPE_T eModelType = AX_ENGINE_MODEL_TYPE0;
        // ret = AX_ENGINE_GetModelType(pModelBufferVirAddr, nModelBufferSize, &eModelType);
        // if (0 != ret || eModelType >= AX_ENGINE_MODEL_TYPE_BUTT) {
        //     printf("%s AX_ENGINE_GetModelType fail ret=%x, eModelType=%d\n", strModelPath.c_str(), eModelType);

        //     freeModelBuffer();

        //     return AX_ERR_ALGO_ILLEGAL_PARAM;
        // }

        // 1.2 Check VNPU
        // AX_ENGINE_NPU_SET_T nNpuSet = 0;
        // ret = CheckModelVNpu(strModelPath, eModelType, 0, nNpuSet);
        // if (0 != ret) {
        //     printf("ALGO CheckModelVNpu fail\n");

        //     freeModelBuffer();

        //     return AX_ERR_ALGO_ILLEGAL_PARAM;
        // }

        // 2. create handle
        AX_ENGINE_HANDLE handle = nullptr;
        // AX_ENGINE_HANDLE_EXTRA_T extra;
        // extra.nNpuSet = nNpuSet;
        // extra.pName = nullptr;

        // ret = AX_ENGINE_CreateHandleV2(&handle, pModelBufferVirAddr, nModelBufferSize, &extra);
        ret = AX_ENGINE_CreateHandle(&handle, pModelBufferVirAddr, nModelBufferSize);
        auto deinit_handle = [&handle]() {
            if (handle) {
                AX_ENGINE_DestroyHandle(handle);
            }
            return AX_ERR_ALGO_ILLEGAL_PARAM;
        };

        freeModelBuffer();

        if (0 != ret || !handle) {
            printf("ALGO Create model(%s) handle fail\n", strModelPath.c_str());

            return deinit_handle();
        }

        // 3. create context
        ret = AX_ENGINE_CreateContext(handle);
        if (0 != ret) {
            return deinit_handle();
        }

        // 4. set io
        m_io_info = nullptr;
        ret = AX_ENGINE_GetIOInfo(handle, &m_io_info);
        if (0 != ret) {
            return deinit_handle();
        }
        m_input_num = m_io_info->nInputSize;
        m_output_num = m_io_info->nOutputSize;

        // for (int i = 0; i < m_io_info->nOutputSize; i++)
        // {
        //     if (m_io_info->pOutputs[i].eLayout == AX_ENGINE_TENSOR_LAYOUT_NHWC)
        //         printf("output[%d]: name: %s  layout: NHWC  data type: %d\n", i, m_io_info->pOutputs[i].pName, m_io_info->pOutputs[i].eDataType);
        //     else
        //         printf("output[%d]: name: %s  layout: NCHW  data type: %d\n", i, m_io_info->pOutputs[i].pName, m_io_info->pOutputs[i].eDataType);
        // }

        // 4.1 query io
        AX_IMG_FORMAT_E eDtype = AX_FORMAT_YUV420_SEMIPLANAR;
        ret = axALGO::query_model_input_size(m_io_info, m_input_size, eDtype);

        if (0 != ret) {
            printf("ALGO model(%s) query model input size fail\n", strModelPath.c_str());
            return deinit_handle();
        }

        if (eDtype == AX_FORMAT_YUV420_SEMIPLANAR || eDtype == AX_FORMAT_RGB888) {
            printf("ALGO model(%s) data type is 0x%02X\n", strModelPath.c_str(), eDtype);
        }
        else {
            printf("ALGO model(%s) data type is: 0x%02X, unsupport\n", strModelPath.c_str(), eDtype);
            return deinit_handle();
        }

        // 4.2 brief io
        #ifdef __AX_ALGO_DEBUG__
            axALGO::brief_io_info(strModelPath, m_io_info);
        #endif

        //5. Config VNPU
        // printf("ALGO model(%s) nNpuSet: 0x%08X\n", strModelPath.c_str(), nNpuSet);
        // will do nothing for using create handle v2 api

        // 6. prepare io
        // AX_U32 nIoDepth = (stCtx.vecOutputBufferFlag.size() == 0) ? 1 : stCtx.vecOutputBufferFlag.size();
        ret = axALGO::prepare_io(strModelPath, m_io_info, m_io, axALGO::ALGO_IO_BUFFER_STRATEGY_CACHED);
        if (0 != ret) {
            printf("prepare io failed!\n");
            return deinit_handle();
        }

        m_handle = handle;
        m_hasInit = true;

        return AX_ALGO_SUCC;
    }

    int EngineWrapper::Preprocess(const AX_VIDEO_FRAME_T& src, AX_VIDEO_FRAME_T& dst, const cv::Rect& crop_rect)
    {
        return utils::CropResizeFrame(src, dst, cv::Size(m_input_size[1], m_input_size[0]), crop_rect);
    }

    int EngineWrapper::Run(const AX_VIDEO_FRAME_T& stFrame)
    {
        if (!m_hasInit)
            return -1;

        // 7.1 fill input & prepare to inference
        auto ret = axALGO::push_io_input(&stFrame, m_io);

        if (0 != ret) {
            printf("push_io_input failed.\n");
            ret = AX_ERR_ALGO_ILLEGAL_PARAM;
            return ret;
        }

        // 7.3 run & benchmark
        {
            ret = AX_ENGINE_RunSync(m_handle, &m_io);
            if (0 != ret) {
                printf("AX_ENGINE_RunSync failed.\n");
                ret = AX_ERR_ALGO_INVALID_HANDLE;
                return ret;
            }
        }

        return ret;
    }

    int EngineWrapper::Release()
    {
        if (m_handle) {
            AX_ENGINE_DestroyHandle(m_handle);
        }
        return AX_ALGO_SUCC;
    }
}