#!/usr/bin/env bash
pwd=`pwd`
thirdparty=${pwd}/thirdparty/RTSP
install_path=${pwd}/thirdparty-install/RTSP

if [[ ! -d ${thirdparty} ]]; then
  echo "RTSP not exist, please run git submodule update"
  exit 1
fi

# RTSP
cd ${thirdparty}
mkdir -p RTSP-build
cd RTSP-build
cmake .. -DCMAKE_INSTALL_PREFIX=${install_path} -DCMAKE_TOOLCHAIN_FILE=${pwd}/toolchains/ax620e.cmake
make -j8
make install
cd ${pwd}