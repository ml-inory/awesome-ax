mkdir -p build
cd build
cmake ../tests -DCMAKE_CXX_COMPILER=aarch64-none-linux-gnu-g++ -DCMAKE_INSTALL_PREFIX=../install
make -j8
make install