#!/bin/bash

echo "Cleaning..."
rm -rf build

echo "Generating and building..."
cmake . -B build && cmake --build build -v
