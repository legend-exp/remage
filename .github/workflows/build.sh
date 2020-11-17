#!/bin/bash

cd ..
mkdir build
cd build
cmake ../remage
make
make install
