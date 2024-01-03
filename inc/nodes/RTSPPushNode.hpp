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

    private:
        void start_server()
        {
            m_server = rtsp_new_server(8554);
            m_session = rtsp_new_session(m_server, "live", 0);
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
            m_nHeight(1080)
        { }

        void set_venc_chn_attr(AX_VENC_CHN_ATTR_S& stVencChnAttr)
        {
            memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));

            stVencChnAttr.stVencAttr.u32MaxPicWidth = 0;
            stVencChnAttr.stVencAttr.u32MaxPicHeight = 0;

            stVencChnAttr.stVencAttr.u32PicWidthSrc = m_nWidth;   /*the picture width*/
            stVencChnAttr.stVencAttr.u32PicHeightSrc = m_nHeight; /*the picture height*/

            stVencChnAttr.stVencAttr.u32CropOffsetX = 0;
            stVencChnAttr.stVencAttr.u32CropOffsetY = 0;
            stVencChnAttr.stVencAttr.u32CropWidth = 0;
            stVencChnAttr.stVencAttr.u32CropHeight = 0;
            stVencChnAttr.stVencAttr.u32VideoRange = 1; /* 0: Narrow Range(NR), Y[16,235], Cb/Cr[16,240]; 1: Full Range(FR), Y/Cb/Cr[0,255] */

            // ALOGN("VencChn %d:w:%d, h:%d, s:%d, Crop:(%d, %d, %d, %d) rcType:%d, payload:%d", gVencChnMapping[VencChn], stVencChnAttr.stVencAttr.u32PicWidthSrc, stVencChnAttr.stVencAttr.u32PicHeightSrc, config.nStride, stVencChnAttr.stVencAttr.u32CropOffsetX, stVencChnAttr.stVencAttr.u32CropOffsetY, stVencChnAttr.stVencAttr.u32CropWidth, stVencChnAttr.stVencAttr.u32CropHeight, config.stRCInfo.eRCType, config.ePayloadType);

            stVencChnAttr.stVencAttr.u32BufSize = m_nWidth * m_nHeight * 3 / 2; /*stream buffer size*/
            stVencChnAttr.stVencAttr.u32MbLinesPerSlice = 0;                                 /*get stream mode is slice mode or frame mode?*/
            stVencChnAttr.stVencAttr.enLinkMode = AX_NOLINK_MODE;
            stVencChnAttr.stVencAttr.u32GdrDuration = 0;
            /* GOP Setting */
            stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;

            stVencChnAttr.stVencAttr.enType = PT_H264;

            stVencChnAttr.stVencAttr.enProfile = VENC_H264_MAIN_PROFILE;
            stVencChnAttr.stVencAttr.enLevel = VENC_H264_LEVEL_5_2;

            AX_VENC_H264_CBR_S stH264Cbr;
            memset(&stH264Cbr, 0, sizeof(stH264Cbr));
            stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
            stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
            stH264Cbr.u32Gop = 50;
            stH264Cbr.u32SrcFrameRate = 30;  /* input frame rate */
            stH264Cbr.fr32DstFrameRate = 30; /* target frame rate */
            stH264Cbr.u32BitRate = m_nWidth * m_nHeight * 3 / 1024;
            stH264Cbr.u32MinQp = 10;
            stH264Cbr.u32MaxQp = 51;
            stH264Cbr.u32MinIQp = 10;
            stH264Cbr.u32MaxIQp = 51;
            stH264Cbr.s32IntraQpDelta = -2;
            memcpy(&stVencChnAttr.stRcAttr.stH264Cbr, &stH264Cbr, sizeof(AX_VENC_H264_CBR_S));
        }

        int Init(const Json::Value& config)
        {
            AddInputPort("frame_input");
            m_session_name = config["rtsp_session"].asCString();

            start_server();

            AX_VENC_MOD_ATTR_S vencAttr;
            memset(&vencAttr, 0, sizeof(AX_VENC_MOD_ATTR_S));
            vencAttr.enVencType = VENC_VIDEO_ENCODER;
            int ret = AX_VENC_Init(&vencAttr);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VENC_Init failed! ret=0x%x\n", ret);
                return ret;
            }

            AX_VENC_CHN_ATTR_S stVencChnAttr;
            set_venc_chn_attr(stVencChnAttr);
            ret = AX_VENC_CreateChn(m_nVencChn, &stVencChnAttr);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VENC_CreateChn failed! ret=0x%x\n", ret);
                return ret;
            }

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

                cv::Mat img = packet.get<cv::Mat>();


                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            printf("[%s]: Stop\n", node_name);
            stop_server();

            return AX_SUCCESS;
        }
    };
}