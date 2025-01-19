#!/bin/bash
set -e

# This script expects to be run from the 'yummyLifeBuildScripts/' directory
cd ../

# Clean up old files
echo ----- CLEANUP -----
echo Removing existing folder/files
rm -rf ./SDL-devel-1.2.15-mingw32.tar.gz
rm -rf ./SDL-1.2.15
rm -rf ./._SDL-1.2.15
rm -rf ./CPP-HTTPLib
rm -rf ./openssl-3.0.15-i686.tar.gz
rm -rf ./openssl-3.0.15-i686
rm -rf ./openssl-3.0.15
echo -------------------
echo

# SDL - Simply download and extract
echo ------- SDL -------
wget https://www.libsdl.org/release/SDL-devel-1.2.15-mingw32.tar.gz
tar zxvf SDL-devel-1.2.15-mingw32.tar.gz
rm SDL-devel-1.2.15-mingw32.tar.gz
echo -------------------
echo

# Cpp-httplib - Simply download the header file, it's all in one ;D
echo --- CPP-HTTPLIB ---
mkdir -p ./CPP-HTTPLib
wget -O ./CPP-HTTPLib/httplib.h https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
echo -------------------
echo

# Download precompiled OpenSSL (32-bit) from olliez-mods/openssl-prebuilt
# Building them yourself takes a long time, so this is faster
echo ----- OPENSSL -----
wget https://github.com/olliez-mods/OpenSSL-Prebuilt/releases/download/3.0.15-i686/openssl-3.0.15-i686.tar.gz
tar zxvf openssl-3.0.15-i686.tar.gz
rm openssl-3.0.15-i686.tar.gz
cp -rf openssl-3.0.15-i686 openssl-3.0.15
rm -rf openssl-3.0.15-i686
echo -------------------
echo