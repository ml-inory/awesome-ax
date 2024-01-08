#!/usr/bin/env bash
pwd=`pwd`
thirdparty=${pwd}/thirdparty/mosquitto
install_path=${pwd}/thirdparty-install/mosquitto

if [[ ! -d ${thirdparty} ]]; then
  echo "mosquitto not exist, please run git submodule update"
  exit 1
fi

# mosquitto
cd ${thirdparty}
mkdir -p mosquitto-build
cd mosquitto-build
cmake .. -DCMAKE_INSTALL_PREFIX=${install_path} -DCMAKE_TOOLCHAIN_FILE=${pwd}/toolchains/ax620e.cmake -DWITH_BUNDLED_DEPS=OF  \
-DWITH_TLS=OFF -DWITH_TLS_PSK=OFF -DWITH_EC=OFF -DWITH_STATIC_LIBRARIES=ON -DWITH_BROKER=OFF -DWITH_APPS=OFF -DWITH_PLUGINS=OFF \
-DDOCUMENTATION=OFF -DBUILD_TESTING=OFF
make -j8
make install
cd ${pwd}