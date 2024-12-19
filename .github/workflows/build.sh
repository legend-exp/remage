#!/bin/bash

mkdir build
cd build || exit 1
cmake ..
make
make install
