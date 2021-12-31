#!/bin/bash

echo "Cleaning..."
rm -rf build install

echo "Building..."
cmake . -B build -DCMAKE_INSTALL_PREFIX=`pwd`/install/ && cmake --build build -t install &&

echo -e "\nBuild done. Installed files:" &&
find install
