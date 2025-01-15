#!/bin/bash
set -e

# YummyLife

# Made to work with WSL and Docker scripts (it is in CRLF format, so be careful with that)
# Does not clear CMakeCache.txt, so it generally runs faster than build-release.sh

# Note for Docker: You may need to have this run twice, first time it runs it might 
# fail because of missing CMakeCache.txt, but it will generate it for the second run.

# Optional params (windows/linux) to only build for specific platforms, pass in nothing to build for both
build_linux=false
build_windows=false
[ $# -eq 0 ] || [ "$1" = "linux" ] && build_linux=true
[ $# -eq 0 ] || [ "$1" = "windows" ] && build_windows=true

echo $# $1
echo $build_windows

# Remove old builds
rm -rf ./devbuild/YummyLife_*

if [ "$build_windows" = true ]; then
  echo ----- Windows -----
  mkdir -p devbuild/windows
  cmake -DCMAKE_TOOLCHAIN_FILE=mingw-cross-toolchain.cmake -B devbuild/windows -S .
  cmake --build devbuild/windows -j
  mv devbuild/windows/YummyLife_windows.exe devbuild/YummyLife_dev_windows.exe
fi

if [ "$build_linux" = true ]; then
  echo ----- Linux -----
  mkdir -p devbuild/linux
  cmake -B devbuild/linux -S .
  cmake --build devbuild/linux -j
  mv devbuild/linux/YummyLife_linux devbuild/YummyLife_dev_linux
fi

# For development, copy to ~/ahap and ~/ohol if folder exists for quick testing
for dst in ahap ohol; do
  if [ -e ~/$dst ]; then
    echo Copying to ~/$dst
    cp devbuild/YummyLife_linux ~/$dst/
    cp devbuild/YummyLife_windows.exe ~/$dst/
  fi
done