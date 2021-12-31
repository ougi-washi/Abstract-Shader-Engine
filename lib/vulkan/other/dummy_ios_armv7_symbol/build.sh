#!/bin/bash

mkdir build
mkdir lib
echo "Compiling..."
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang -c -target armv7-apple-ios9.0 src/dummy.c -o build/dummy.o
echo "Creating archive..."
ar rvs lib/dummy.a build/dummy.o
echo "Done."
