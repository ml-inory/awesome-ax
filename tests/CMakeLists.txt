cmake_minimum_required(VERSION 3.15)

project(TESTS CXX)

include_directories(../inc)

set(THIRDPARTY ../thirdparty-install)

set(JSONCPP ${THIRDPARTY}/jsoncpp)
set(OPENCV ${THIRDPARTY}/opencv)
set(msp ${THIRDPARTY}/msp/out)
set(RTSP ${THIRDPARTY}/RTSP)
set(RtspServer ${THIRDPARTY}/RtspServer)

include_directories(${JSONCPP}/include)
include_directories(${OPENCV}/include/opencv4)
include_directories(${msp}/include)
include_directories(${RTSP}/include)
include_directories(${RtspServer}/include)

link_directories(${JSONCPP}/lib)
link_directories(${OPENCV}/lib)
link_directories(${OPENCV}/lib/opencv4/3rdparty/)
link_directories(${msp}/lib)
link_directories(${RTSP}/lib)
link_directories(${RtspServer}/lib)

list(APPEND AX_LIBS ax_mipi
        ax_nt_ctrl
        ax_nt_stream
        ax_proton
        ax_engine
        ax_interpreter
        ax_ae
        ax_awb
        ax_af
        ax_venc
        ax_vdec
        ax_ivps
        ax_sys)

list(APPEND LIBS libjsoncpp.a
        opencv_core
        opencv_imgproc
        opencv_imgcodecs
        ittnotify
        tegra_hal
        libjpeg-turbo
        libpng
        zlib
        rtspclisvr
        RtspServer
        pthread
        dl
        rt
        m)

add_executable(test_rtsp test_rtsp.cpp)
target_link_libraries(test_rtsp ${LIBS} ${AX_LIBS})

install(TARGETS test_rtsp
        RUNTIME DESTINATION bin)