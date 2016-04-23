#!/bin/bash

cd /vagrant
rm -rf cpp-netlib-0.9.4
wget https://cloud.github.com/downloads/cpp-netlib/cpp-netlib/cpp-netlib-0.9.4.zip
unzip cpp-netlib-0.9.4.zip
rm -rf cpp-netlib-0.9.4.zip
cd cpp-netlib-0.9.4
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
make
