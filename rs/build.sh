#!/bin/sh

mkdir -p ../build
cd ../build
cmake ..
make -j4 install

