#! /bin/bash
mkdir -p build
cmake -DCMAKE_BUILD_TYPE=Debug -B build -S .
make -C build
