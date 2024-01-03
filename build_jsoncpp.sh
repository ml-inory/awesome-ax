#!/usr/bin/env bash
pwd=`pwd`
thirdparty=${pwd}/thirdparty/jsoncpp
install_path=${pwd}/thirdparty-install/jsoncpp

if [[ ! -d ${thirdparty} ]]; then
  echo "jsoncpp not exist, please run git submodule update"
  exit 1
fi

# jsoncpp
cd ${thirdparty}
mkdir jsoncpp-build
cd jsoncpp-build
cmake .. -DCMAKE_INSTALL_PREFIX=${install_path} -DCMAKE_TOOLCHAIN_FILE=${pwd}/toolchains/ax620e.cmake -DJSONCPP_WITH_TESTS=OFF -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF -DJSONCPP_WITH_EXAMPLE=OFF
make -j8
make install
cd ${pwd}