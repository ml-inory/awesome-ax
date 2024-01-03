#!/usr/bin/env bash
mkdir third-party-install
cd third-party-install
wget https://github.com/ZHEQIUSHUI/assets/releases/download/ax650/libopencv-4.5.5-aarch64.zip
unzip libopencv-4.5.5-aarch64.zip
rm opencv-arm-linux-gnueabihf-gcc-7.5.0.zip
cd ..