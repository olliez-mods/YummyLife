#!/bin/bash

# This script expects to be run from the 'yummyLifeBuildScripts/' directory
cd ../

# Clean up old files
echo --- CLEANUP ---
echo
rm -rf ./SDL-1.2.15
rm -rf ./._SDL-1.2.15
rm -rf ./libcurl-source
rm -rf ./curl-7.79.1
echo ---------------
echo

# SDL - Simply download and extract
echo ----- SDL -----
curl -O https://www.libsdl.org/release/SDL-devel-1.2.15-mingw32.tar.gz
tar zxvf SDL-devel-1.2.15-mingw32.tar.gz
rm SDL-devel-1.2.15-mingw32.tar.gz
echo ---------------
echo

# LibCurl - Needs to be built on the system
echo --- LIBCURL ---
curl -O https://curl.se/download/curl-7.79.1.tar.gz
tar zxvf curl-7.79.1.tar.gz
rm curl-7.79.1.tar.gz
mkdir -p ./libcurl-7.79.1
cd curl-7.79.1
./configure --prefix=$(pwd)/../libcurl-7.79.1 --with-ssl
make && make install
cd ..
rm -rf curl-7.79.1
echo ---------------
echo
