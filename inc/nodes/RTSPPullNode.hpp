#pragma once

#include <cstring>

#include "node.hpp"
#include "rtspclisvr/RTSPClient.h"

#include "ax_sys_api.h"
#include "ax_vdec_api.h"

#include "opencv2/opencv.hpp"
#include "ax_buffer_tool.h"

// 16字节对齐
#define ALIGN_16(x)     ((x + 15) / 16 * 16)

//FILE* fp_dump = NULL;

AX_S32 FramePoolInit(AX_VDEC_GRP VdGrp, AX_U32 FrameSize, AX_POOL *PoolId, AX_U32 u32FrameBufCnt)
{
    AX_S32 s32Ret = AX_SUCCESS;
    /* vdec use pool to alloc output buffer */
    AX_POOL_CONFIG_T stPoolConfig = {0};
    AX_POOL s32PoolId;

    memset(&stPoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
    stPoolConfig.MetaSize = 512;
    stPoolConfig.BlkCnt = u32FrameBufCnt;
    stPoolConfig.BlkSize = FrameSize;
    stPoolConfig.CacheMode = AX_POOL_CACHE_MODE_NONCACHE;
    memset(stPoolConfig.PartitionName, 0, sizeof(stPoolConfig.PartitionName));
    strcpy((AX_CHAR *)stPoolConfig.PartitionName, "anonymous");

    s32PoolId = AX_POOL_CreatePool(&stPoolConfig);
    if (AX_INVALID_POOLID == s32PoolId)
    {
        printf("Create pool err.\n");
        return AX_ERR_VDEC_NULL_PTR;
    }

    *PoolId = s32PoolId;

    s32Ret = AX_VDEC_AttachPool(VdGrp, s32PoolId);
    if (s32Ret != AX_SUCCESS)
    {
        AX_POOL_DestroyPool(s32PoolId);
        printf("Attach pool err. 0x%x\n", s32Ret);
    }

    printf("FramePoolInit successfully cnt %d size %#x! %d\n", s32PoolId, u32FrameBufCnt, FrameSize);

    return s32Ret;
}

namespace ax
{
    class RTSPPullNode : public Node
    {
    private:
        RTSPClient *m_client;
        const char* m_rtspUrl;

        // 解码参数
        const int nVdecGrp = 0;
        AX_POOL s32PoolId = 0;
        const int nPicWidth = 1920;
        const int nPicHeight = 1080;
        AX_U64 m_nPts;

    public:
        RTSPPullNode():
            Node("RTSP_Pull"),
            m_nPts(0)
        { }

        ~RTSPPullNode()
        {
//            fclose(fp_dump);
        }

        int Init(const Json::Value& config)
        {
            AddOutputPort("frame_output");
            m_rtspUrl = config["rtsp_url"].asCString();

            // 打开VDEC
            if (OpenVDEC() != AX_SUCCESS)
            {
                printf("open vdec failed!\n");
                return AX_ERR_INIT_FAIL;
            }

            // open client
            m_client = new RTSPClient;
            if (m_client->openURL(m_rtspUrl, 1) != 0)
            {
                printf("open url: %s falied!\n", m_rtspUrl);
                return AX_ERR_INIT_FAIL;
            }

            // play
            if (m_client->playURL(frameHandlerFunc, this, NULL, NULL) != 0)
            {
                printf("play url: %s falied!\n", m_rtspUrl);
                return AX_ERR_INIT_FAIL;
            }

//            fp_dump = fopen("test.h264", "wb");

            m_hasInit = true;
            return AX_SUCCESS;
        }

        int OpenVDEC()
        {
            int ret = AX_SUCCESS;

            // 初始化VDEC
            AX_VDEC_MOD_ATTR_T stVdecModAttr;
            stVdecModAttr.u32MaxGroupCount = 0;
            ret = AX_VDEC_Init(&stVdecModAttr);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_Init failed! ret=0x%x\n", ret);
                return ret;
            }

            // 创建解码通道
            AX_VDEC_GRP_ATTR_T stGrpAttr;
            memset(&stGrpAttr, 0, sizeof(AX_VDEC_GRP_ATTR_T));
            stGrpAttr.enCodecType = PT_H264;
            stGrpAttr.enInputMode = AX_VDEC_INPUT_MODE_FRAME;
            stGrpAttr.enLinkMode = AX_UNLINK_MODE;
            stGrpAttr.u32PicWidth = 1920;
            stGrpAttr.u32PicHeight = 1920;
            stGrpAttr.u32FrameHeight = 0;
            stGrpAttr.u32StreamBufSize = 1 * 1024 * 1024;
            stGrpAttr.u32FrameBufCnt = 30;
            stGrpAttr.s32DestroyTimeout = 0;
            stGrpAttr.enOutOrder = AX_VDEC_OUTPUT_ORDER_DISP;
            stGrpAttr.enVdecVbSource = AX_POOL_SOURCE_USER;

            ret = AX_VDEC_CreateGrp(nVdecGrp, &stGrpAttr);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_CreateGrp failed! ret=0x%x\n", ret);
                return ret;
            }

            AX_U32 FrameSize = 0;

            FrameSize = AX_VDEC_GetPicBufferSize(1920, 1920, PT_H264); // 3655712;
            printf("Get pool mem size is %d\n", FrameSize);
            ret = FramePoolInit(nVdecGrp, FrameSize, &s32PoolId, stGrpAttr.u32FrameBufCnt);
            if (ret != AX_SUCCESS)
            {
                printf("FramePoolInit failed! Error:%x\n", ret);
                return ret;
            }

            ret = AX_VDEC_AttachPool(nVdecGrp, s32PoolId);
            if (ret != AX_SUCCESS)
            {
                AX_POOL_DestroyPool(s32PoolId);
                printf("Attach pool err. %x\n", ret);
                return ret;
            }

            AX_VDEC_GRP_PARAM_T stGrpParam;
            ret = AX_VDEC_GetGrpParam(nVdecGrp, &stGrpParam);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_GetGrpParam failed! 0x%x\n", ret);
                return -1;
            }

            stGrpParam.enVdecMode = VIDEO_DEC_MODE_IPB;
            ret = AX_VDEC_SetGrpParam(nVdecGrp, &stGrpParam);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_SetGrpParam failed! 0x%x\n", ret);
                return -1;
            }

            ret = AX_VDEC_SetDisplayMode(nVdecGrp, AX_VDEC_DISPLAY_MODE_PLAYBACK);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_SetDisplayMode failed! ret=0x%x\n", ret);
                return ret;
            }

            // 开始接收码流
            AX_VDEC_RECV_PIC_PARAM_T stRecvParam = {0};
            ret = AX_VDEC_StartRecvStream(nVdecGrp, &stRecvParam);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_StartRecvStream failed! ret=0x%x\n", ret);
                return ret;
            }

            return AX_SUCCESS;
        }

        void CloseVDEC()
        {
            int ret = AX_SUCCESS;

            AX_VDEC_StopRecvStream(nVdecGrp);
            AX_VDEC_DetachPool(nVdecGrp);
            AX_POOL_DestroyPool(s32PoolId);

            // 销毁解码通道
            ret = AX_VDEC_DestroyGrp(nVdecGrp);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_DestroyGrp failed! ret=0x%x\n", ret);
            }

            // 关闭VDEC
            ret = AX_VDEC_Deinit();
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_Deinit failed! ret=0x%x\n", ret);
            }
        }

        static void frameHandlerFunc(void *arg, RTP_FRAME_TYPE frame_type, int64_t timestamp, unsigned char *buf, int len)
        {
            RTSPPullNode* node = (RTSPPullNode*)arg;
            switch (frame_type)
            {
            case FRAME_TYPE_VIDEO:
//                printf("Received stream %d\n", len);
//                fwrite(buf, len, 1, fp_dump);
                node->SendStream(buf, len);
                break;
            case FRAME_TYPE_AUDIO:
                break;
            case FRAME_TYPE_ETC:
                break;
            default:
                break;
            }
        }

        int SendStream(unsigned char* buf, int len)
        {
            int ret = AX_SUCCESS;
            AX_VDEC_STREAM_T stream;
            memset(&stream, 0, sizeof(AX_VDEC_STREAM_T));
            stream.u64PTS = m_nPts++;
            stream.pu8Addr = buf;
            stream.u32StreamPackLen = len;
            stream.u64PhyAddr = 0;
            stream.bEndOfFrame = AX_FALSE;
            stream.bEndOfStream = AX_FALSE;
            ret = AX_VDEC_SendStream(nVdecGrp, &stream, -1);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_SendStream failed! ret=0x%x\n", ret);
                return ret;
            }
            return AX_SUCCESS;
        }

        int Run()
        {
            const char* node_name = m_name.c_str();
            printf("[%s]: %s start\n", node_name, node_name);

            auto frame_output_port = FindOutputPort("frame_output");

            int ret = AX_SUCCESS;
            while (m_isRunning)
            {
                // 获取帧
                AX_VIDEO_FRAME_INFO_T stFrameInfo;
                ret = AX_VDEC_GetFrame(nVdecGrp, &stFrameInfo, -1);
                if (ret != AX_SUCCESS)
                {
//                    printf("AX_VDEC_GetFrame failed! ret=0x%x\n", ret);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

                stFrameInfo.stVFrame.u64VirAddr[0] = (AX_U64)AX_POOL_GetBlockVirAddr(stFrameInfo.stVFrame.u32BlkId[0]);
                stFrameInfo.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(stFrameInfo.stVFrame.u32BlkId[0]);

                AX_VIDEO_FRAME_T output_frame = {0};
                memcpy(&output_frame, &stFrameInfo.stVFrame, sizeof(AX_VIDEO_FRAME_T));
                frame_output_port->send(Packet(output_frame));

                // 释放帧
                ret = AX_VDEC_ReleaseFrame(nVdecGrp, &stFrameInfo);
                if (ret != AX_SUCCESS)
                {
                    printf("AX_VDEC_ReleaseFrame failed! ret=0x%x\n", ret);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            printf("[%s]: Stop\n", node_name);
            if (m_client)
            {
                m_client->closeURL();
                delete m_client;
            }
            CloseVDEC();

            return AX_SUCCESS;
        }
    };
}
