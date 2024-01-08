mkdir -p build
cd build
cmake ../tests -DCMAKE_TOOLCHAIN_FILE=../toolchains/ax620e.cmake -DCMAKE_INSTALL_PREFIX=../install
make -j8
make install