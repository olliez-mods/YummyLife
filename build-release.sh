#!/bin/bash
set -e

rm -rf relbuild
mkdir -p relbuild/{windows,linux}

preview_flag=""
if [ "${PREVIEW_BUILD:-false}" = "true" ]; then
  preview_flag="-DPREVIEW_BUILD=ON"
fi

echo ----- Windows -----
cmake -DCMAKE_TOOLCHAIN_FILE=mingw-cross-toolchain.cmake -B relbuild/windows -S . ${preview_flag}
cmake --build relbuild/windows -j

echo ----- Linux -----
cmake -B relbuild/linux -S . ${preview_flag}
cmake --build relbuild/linux -j

mv relbuild/linux/YummyLife_linux relbuild/
mv relbuild/windows/YummyLife_windows.exe relbuild/

for dst in ahap ohol; do
  if [ -e ~/$dst ]; then
    echo Copying to ~/$dst
    cp relbuild/YummyLife_linux ~/$dst/
    cp relbuild/YummyLife_windows.exe ~/$dst/
  fi
done

rm -rf relbuild/{linux,windows}
