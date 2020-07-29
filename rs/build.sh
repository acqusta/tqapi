#!/bin/sh

mkdir -p ../build-linux
cd ../build-linux
cmake ..
make -j4 install

