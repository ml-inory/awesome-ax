#pragma once

#include <string.h>

#include "node.hpp"
#include "libRtspServer/RtspServerWarpper.h"

#include "ax_sys_api.h"
#include "ax_venc_api.h"

#include "opencv2/opencv.hpp"

// 16字节对齐
#define ALIGN_16(x)     ((x + 15) / 16 * 16)

namespace ax
{
    class RTSPPushNode : public Node
    {
    private:
        rtsp_server_t m_server;
        rtsp_session_t m_session;
        const char* m_session_name;
        int m_nVencChn;
        int m_nWidth, m_nHeight;
        AX_U64 m_nPts;

    private:
        void start_server()
        {
            m_server = rtsp_new_server(8554);
            m_session = rtsp_new_session(m_server, "axstream", 0);
        }

        void stop_server()
        {
            rtsp_rel_session(m_server, m_session);
            rtsp_rel_server(&m_server);
        }

    public:
        RTSPPushNode():
            Node("RTSP_Push"),
            m_server(nullptr),
            m_session(nullptr),
            m_nVencChn(0),
            m_nWidth(1920),
            m_nHeight(1080),
            m_nPts(0)
        { }

        ~RTSPPushNode()
        {
            if (m_hasInit)
            {
                stop_server();
            }
        }

        void set_venc_chn_attr(AX_VENC_CHN_ATTR_T& stVencChnAttr)
        {
            memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_T));

            stVencChnAttr.stVencAttr.u8InFifoDepth = 1;
            stVencChnAttr.stVencAttr.u8OutFifoDepth = 1;

            stVencChnAttr.stVencAttr.u32MaxPicWidth = m_nWidth;
            stVencChnAttr.stVencAttr.u32MaxPicHeight = m_nHeight;

            stVencChnAttr.stVencAttr.u32PicWidthSrc = m_nWidth;   /*the picture width*/
            stVencChnAttr.stVencAttr.u32PicHeightSrc = m_nHeight; /*the picture height*/

            // ALOGN("VencChn %d:w:%d, h:%d, s:%d, Crop:(%d, %d, %d, %d) rcType:%d, payload:%d", gVencChnMapping[VencChn], stVencChnAttr.stVencAttr.u32PicWidthSrc, stVencChnAttr.stVencAttr.u32PicHeightSrc, config.nStride, stVencChnAttr.stVencAttr.u32CropOffsetX, stVencChnAttr.stVencAttr.u32CropOffsetY, stVencChnAttr.stVencAttr.u32CropWidth, stVencChnAttr.stVencAttr.u32CropHeight, config.stRCInfo.eRCType, config.ePayloadType);

            stVencChnAttr.stVencAttr.u32BufSize = m_nWidth * m_nHeight * 3 / 2; /*stream buffer size*/
            stVencChnAttr.stVencAttr.enLinkMode = AX_UNLINK_MODE;
            /* GOP Setting */
            stVencChnAttr.stGopAttr.enGopMode = AX_VENC_GOPMODE_NORMALP;

            stVencChnAttr.stVencAttr.enType = PT_H264;

            stVencChnAttr.stVencAttr.enProfile = AX_VENC_H264_MAIN_PROFILE;
            stVencChnAttr.stVencAttr.enLevel = AX_VENC_H264_LEVEL_5_2;

            stVencChnAttr.stRcAttr.enRcMode = AX_VENC_RC_MODE_H264CBR;
            stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
            stVencChnAttr.stRcAttr.stFrameRate.fSrcFrameRate = 25.0f;
            stVencChnAttr.stRcAttr.stFrameRate.fDstFrameRate = 25.0f;

            AX_VENC_H264_CBR_T stH264Cbr;
            memset(&stH264Cbr, 0, sizeof(stH264Cbr));
            stH264Cbr.u32Gop = 50;
            stH264Cbr.u32BitRate = m_nWidth * m_nHeight * 3 / 1024;
            stH264Cbr.u32MinQp = 10;
            stH264Cbr.u32MaxQp = 51;
            stH264Cbr.u32MinIQp = 10;
            stH264Cbr.u32MaxIQp = 51;
            stH264Cbr.s32IntraQpDelta = -2;
            stH264Cbr.u32MaxIprop = 40;
            stH264Cbr.u32MinIprop = 10;

            memcpy(&stVencChnAttr.stRcAttr.stH264Cbr, &stH264Cbr, sizeof(AX_VENC_H264_CBR_T));
        }

        int Init(const Json::Value& config)
        {
            AddInputPort("frame_input");
            m_session_name = config["rtsp_session"].asCString();

            start_server();

            AX_VENC_MOD_ATTR_T vencAttr;
            memset(&vencAttr, 0, sizeof(AX_VENC_MOD_ATTR_T));
            vencAttr.enVencType = AX_VENC_VIDEO_ENCODER ;
            vencAttr.stModThdAttr.u32TotalThreadNum = 2;
            vencAttr.stModThdAttr.bExplicitSched = AX_FALSE;
            int ret = AX_VENC_Init(&vencAttr);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VENC_Init failed! ret=0x%x\n", ret);
                return ret;
            }

            AX_VENC_CHN_ATTR_T stVencChnAttr;
            set_venc_chn_attr(stVencChnAttr);
            ret = AX_VENC_CreateChn(m_nVencChn, &stVencChnAttr);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VENC_CreateChn failed! ret=0x%x\n", ret);
                return ret;
            }

            AX_VENC_RECV_PIC_PARAM_T stRecvParam;
            stRecvParam.s32RecvPicNum = 0;
            ret = AX_VENC_StartRecvFrame(m_nVencChn, &stRecvParam);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VENC_StartRecvFrame failed! ret=0x%x\n", ret);
                return ret;
            }

            m_hasInit = true;
            return AX_SUCCESS;
        }

        int Run()
        {
            const char* node_name = m_name.c_str();
            printf("[%s]: %s start\n", node_name, node_name);

            auto frame_input_port = FindInputPort("frame_input");

            int ret = AX_SUCCESS;
            while (m_isRunning)
            {
                Packet packet;
                if (AX_SUCCESS != frame_input_port->recv(packet))
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                AX_VIDEO_FRAME_T input_frame = packet.get<AX_VIDEO_FRAME_T>();
                AX_VIDEO_FRAME_INFO_T input_frame_info;
                memset(&input_frame_info, 0, sizeof(AX_VIDEO_FRAME_INFO_T));
                memcpy(&input_frame_info.stVFrame, &input_frame, sizeof(AX_VIDEO_FRAME_T));

                ret = AX_VENC_SendFrame(m_nVencChn, &input_frame_info, -1);
                if (ret != AX_SUCCESS) {
                    printf("AX_VENC_SendFrame failed! ret=0x%x\n", ret);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

                AX_VENC_STREAM_T stStream = {0};
                ret = AX_VENC_GetStream(m_nVencChn, &stStream, -1);
                if (ret != AX_SUCCESS) {
                    printf("AX_VENC_GetStream failed! ret=0x%x\n", ret);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

                rtsp_buffer_t buff = {0};
                buff.vts = m_nPts++;
                buff.vbuff = stStream.stPack.pu8Addr;
                buff.vlen = stStream.stPack.u32Len;
                ret = rtsp_push(m_server, m_session, &buff);
                if (ret != 0)
                {
                    printf("rtsp_push failed!\n");
                }

                ret = AX_VENC_ReleaseStream(m_nVencChn, &stStream);
                if (ret != AX_SUCCESS) {
                    printf("AX_VENC_ReleaseStream failed! ret=0x%x\n", ret);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            printf("[%s]: Stop\n", node_name);
            AX_VENC_StopRecvFrame(m_nVencChn);
            AX_VENC_DestroyChn(m_nVencChn);
            AX_VENC_Deinit();

            return AX_SUCCESS;
        }
    };
}