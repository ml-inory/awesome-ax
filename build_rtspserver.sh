#!/usr/bin/env bash
pwd=`pwd`
thirdparty=${pwd}/thirdparty/RtspServer
install_path=${pwd}/thirdparty-install/RtspServer

if [[ ! -d ${thirdparty} ]]; then
  echo "RtspServer not exist, please run git submodule update"
  exit 1
fi

# RTSP
cd ${thirdparty}
mkdir -p RtspServer-build
cd RtspServer-build
cmake .. -DCMAKE_INSTALL_PREFIX=${install_path} -DCMAKE_TOOLCHAIN_FILE=${pwd}/toolchains/ax620e.cmake
make -j8
make install
cd ${pwd}