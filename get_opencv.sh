#!/usr/bin/env bash
mkdir thirdparty-install
cd thirdparty-install
wget https://github.com/ZHEQIUSHUI/assets/releases/download/ax650/libopencv-4.5.5-aarch64.zip
unzip libopencv-4.5.5-aarch64.zip -d opencv
rm libopencv-4.5.5-aarch64.zip
cd ..