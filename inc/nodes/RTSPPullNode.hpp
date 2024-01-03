#pragma once

#include <string.h>

#include "node.hpp"
#include "rtspclisvr/RTSPClient.h"

#include "ax_sys_api.h"
#include "ax_vdec_api.h"

#include "opencv2/opencv.hpp"

// 16字节对齐
#define ALIGN_16(x)     ((x + 15) / 16 * 16)

namespace ax
{
    class RTSPPullNode : public Node
    {
    private:
        RTSPClient m_client;
        const char* m_rtspUrl;

        // 解码参数
        const int nVdecGrp = 0;
        const int nPicWidth = 1280;
        const int nPicHeight = 720;

    public:
        RTSPPullNode():
            Node("RTSP_Pull")
        { }

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
            if (m_client.openURL(m_rtspUrl, 1) != 0)
            {
                printf("open url: %s falied!\n", m_rtspUrl);
                return AX_ERR_INIT_FAIL;
            }

            // play
            if (m_client.playURL(frameHandlerFunc, this, NULL, NULL) != 0)
            {
                printf("play url: %s falied!\n", m_rtspUrl);
                return AX_ERR_INIT_FAIL;
            }
            return AX_SUCCESS;
        }

        int OpenVDEC()
        {
            int ret = AX_SUCCESS;

            // 初始化VDEC
            ret = AX_VDEC_Init();
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_Init failed! ret=0x%x\n", ret);
                return ret;
            }

            // 创建解码通道
            AX_VDEC_GRP_ATTR_S stGrpAttr;
            memset(&stGrpAttr, 0, sizeof(AX_VDEC_GRP_ATTR_S));
            stGrpAttr.enType = PT_H264;
            stGrpAttr.enMode = VIDEO_MODE_STREAM;
            stGrpAttr.enLinkMode = AX_NONLINK_MODE;
            stGrpAttr.u32PicWidth = ALIGN_16(nPicWidth);
            stGrpAttr.u32PicHeight = ALIGN_16(nPicHeight);
            stGrpAttr.u32FrameHeight = nPicHeight;
            stGrpAttr.u32StreamBufSize = nPicHeight * nPicWidth * 3 / 2;
            stGrpAttr.u32FrameBufCnt = 10;
            stGrpAttr.s32DestroyTimeout = 0;
            stGrpAttr.stVdecVideoAttr.eOutOrder = VIDEO_OUTPUT_ORDER_DISP;

            ret = AX_VDEC_CreateGrp(nVdecGrp, &stGrpAttr);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_CreateGrp failed! ret=0x%x\n", ret);
                return ret;
            }

            // 开始接收码流
            ret = AX_VDEC_StartRecvStream(nVdecGrp);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_StartRecvStream failed! ret=0x%x\n", ret);
                return ret;
            }

            return AX_SUCCESS;
        }

        void CloseGVDEC()
        {
            int ret = AX_SUCCESS;

            // 销毁解码通道
            ret = AX_VDEC_DestroyGrp(nVdecGrp);
            if (ret != AX_SUCCESS)
            {
                printf("AX_VDEC_DestroyGrp failed! ret=0x%x\n", ret);
            }

            // 关闭VDEC
            ret = AX_VDEC_DeInit();
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
            AX_VDEC_STREAM_S stream;
            memset(&stream, 0, sizeof(AX_VDEC_STREAM_S));
            stream.pu8Addr = buf;
            stream.u32Len = len;
            ret = AX_VDEC_SendStream(nVdecGrp, &stream, 100);
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
                AX_VIDEO_FRAME_INFO_S stFrameInfo;
                ret = AX_VDEC_GetFrame(nVdecGrp, &stFrameInfo, 100);
                if (ret != AX_SUCCESS)
                {
                    printf("AX_VDEC_GetFrame failed! ret=0x%x\n", ret);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

            
                int width = stFrameInfo.stVFrame.u32Width;
                int height = stFrameInfo.stVFrame.u32FrameSize * 2 / 3 / width;

                cv::Mat img(height, width, CV_8UC1);
                memcpy(img.data, (void*)stFrameInfo.stVFrame.u64VirAddr[0], stFrameInfo.stVFrame.u32FrameSize);
                
                frame_output_port->send(Packet(img));

                // 释放帧
                ret = AX_VDEC_ReleaseFrame(nVdecGrp, &stFrameInfo);
                if (ret != AX_SUCCESS)
                {
                    printf("AX_VDEC_ReleaseFrame failed! ret=0x%x\n", ret);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            printf("[%s]: Stop\n", node_name);
            m_client.closeURL();
            CloseGVDEC();

            return AX_SUCCESS;
        }
    };
}
