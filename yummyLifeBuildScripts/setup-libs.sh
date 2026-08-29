#!/bin/bash
set -e

# This script expects to be run from the 'yummyLifeBuildScripts/' directory
cd ../

# macOS gets a different set of libraries: SDL and OpenSSL come from Homebrew
# instead of the Windows cross-compiling prebuilts, and it has curl but no wget.
host_os=$(uname -s)

download() {
  # download <url> [output-path]
  if [ -n "$2" ]; then
    curl -L -o "$2" "$1"
  else
    curl -L -O "$1"
  fi
}

# Clean up old files
echo ----- CLEANUP -----
echo Removing existing folder/files
rm -rf ./SDL-devel-1.2.15-mingw32.tar.gz
rm -rf ./CPP-HTTPLib
rm -rf ./nlohmann
if [ "$host_os" != "Darwin" ]; then
  rm -rf ./SDL-1.2.15
  rm -rf ./._SDL-1.2.15
  rm -rf ./openssl-3.0.15-i686.tar.gz
  rm -rf ./openssl-3.0.15-i686
  rm -rf ./openssl-3.0.15
  rm -rf ./zlib-1.3.1.tar.gz
  rm -rf ./zlib-1.3.1
  rm -rf ./libpng-1.6.44.tar.gz
  rm -rf ./libpng-1.6.44
fi
echo -------------------
echo

if [ "$host_os" = "Darwin" ]; then
  # SDL 1.2 (via sdl12-compat) and OpenSSL come from Homebrew on macOS
  echo ------ BREW -------
  if ! command -v brew > /dev/null; then
    echo "Homebrew is required for the macOS setup, install it from https://brew.sh" >&2
    exit 1
  fi
  # libpng is only needed by the editor target
  brew install cmake sdl12-compat openssl@3 libpng
  echo -------------------
  echo
else
  # SDL - Simply download and extract
  echo ------- SDL -------
  download https://www.libsdl.org/release/SDL-devel-1.2.15-mingw32.tar.gz
  tar zxvf SDL-devel-1.2.15-mingw32.tar.gz
  rm SDL-devel-1.2.15-mingw32.tar.gz
  echo -------------------
  echo
fi

# Cpp-httplib - Simply download the header file, it's all in one ;D
echo --- CPP-HTTPLIB ---
mkdir -p ./CPP-HTTPLib
download https://raw.githubusercontent.com/yhirose/cpp-httplib/v0.14.3/httplib.h ./CPP-HTTPLib/httplib.h
echo -------------------
echo

if [ "$host_os" != "Darwin" ]; then
  # Download precompiled OpenSSL (32-bit) from olliez-mods/openssl-prebuilt
  # Building them yourself takes a long time, so this is faster
  echo ----- OPENSSL -----
  download https://github.com/olliez-mods/OpenSSL-Prebuilt/releases/download/3.0.15-i686/openssl-3.0.15-i686.tar.gz
  tar zxvf openssl-3.0.15-i686.tar.gz
  rm openssl-3.0.15-i686.tar.gz
  cp -rf openssl-3.0.15-i686 openssl-3.0.15
  rm -rf openssl-3.0.15-i686
  echo -------------------
  echo
fi

if [ "$host_os" != "Darwin" ]; then
  # zlib and libpng, for the Windows editor. Sources rather than a prebuilt
  echo ------- PNG -------
  download https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz
  tar zxf zlib-1.3.1.tar.gz
  rm zlib-1.3.1.tar.gz

  download https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.44.tar.gz ./libpng-1.6.44.tar.gz
  tar zxf libpng-1.6.44.tar.gz
  rm libpng-1.6.44.tar.gz
  # libpng normally generates this from its configure run; the shipped copy is
  # the stock configuration, which is what we want
  cp libpng-1.6.44/scripts/pnglibconf.h.prebuilt libpng-1.6.44/pnglibconf.h
  echo -------------------
  echo
fi

# Download single header file for nlohmann/json
echo -- NLOHMANN/JSON --
mkdir -p ./nlohmann
download https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp ./nlohmann/json.hpp
echo -------------------
echo
