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

#include "ax_engine_api.h"

#include <string>
#include <vector>
#include <string.h>
#include <array>
#include <ax_global_type.h>

#include "opencv2/core.hpp"

namespace infer
{
    #define BLOB_NAME_LEN       32

    struct Blob
    {
        int size;
        void* data;
        char name[BLOB_NAME_LEN];

        Blob():
            size(0),
            data(nullptr)
        { }

        Blob(const char* name_, int size_, void* data_)
        {
            snprintf(name, BLOB_NAME_LEN, "%s", name_);
            size = size_;
            data = new char[size];
            memcpy(data, data_, size);
        }

        Blob(const Blob& other)
        {
            if (&other != this)
            {
                snprintf(name, BLOB_NAME_LEN, "%s", other.name);
                size = other.size;
                data = new char[size];
                memcpy(data, other.data, size);
            }
        }

        Blob(Blob&& other)
        {
            snprintf(name, BLOB_NAME_LEN, "%s", other.name);
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }

        Blob& operator=(const Blob& other)
        {
            if (this == &other)
                return *this;

            if (data)
                delete[] (char*)data;
            snprintf(name, BLOB_NAME_LEN, "%s", other.name);
            size = other.size;
            data = new char[size];
            memcpy(data, other.data, size);
            return *this;
        }

        ~Blob()
        {
            if (data)
                delete[] (char*)data;
        }
    };


    class EngineWrapper
    {
    public:
        EngineWrapper():
            m_hasInit(false),
            m_handle(nullptr)
        { }
        
        ~EngineWrapper() = default;

        int Init(const std::string& strModelPath);
        
        /// @brief Default preprocess: resize to m_input_size
        /// @param src 
        /// @param dst 
        /// @return 
        int Preprocess(const AX_VIDEO_FRAME_T& src, AX_VIDEO_FRAME_T& dst, const cv::Rect& crop_rect = cv::Rect());

        int Run(const AX_VIDEO_FRAME_T& stFrame);

        int Release();

        inline std::array<int, 2> GetInputSize() const { return m_input_size; }

    protected:
        bool m_hasInit;
        std::array<int, 2> m_input_size;
        AX_ENGINE_HANDLE m_handle;
        AX_ENGINE_IO_INFO_T* m_io_info;
        AX_ENGINE_IO_T m_io;
        int m_input_num, m_output_num;
    };
}