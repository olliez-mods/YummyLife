#!/bin/bash
set -e

# Optional params (windows/linux/macos) to only build for specific platforms, pass in
# nothing to build everything this host can produce.
# macOS binaries can only be built on a Mac (Docker/WSL cannot cross-compile them),
# and the Linux/Windows builds are the ones the Docker image provides.
host_os=$(uname -s)

build_linux=false
build_windows=false
build_macos=false

if [ $# -eq 0 ]; then
  if [ "$host_os" = "Darwin" ]; then
    build_macos=true
  else
    build_linux=true
    build_windows=true
  fi
else
  for target in "$@"; do
    case "$target" in
      linux)      build_linux=true ;;
      windows)    build_windows=true ;;
      macos|mac)  build_macos=true ;;
      *)
        echo "Unknown target '$target' (expected: linux, windows, macos)" >&2
        exit 1
        ;;
    esac
  done
fi

if [ "$build_macos" = true ] && [ "$host_os" != "Darwin" ]; then
  echo "The macOS build has to be run natively on a Mac, Docker/WSL cannot produce it" >&2
  exit 1
fi
if { [ "$build_linux" = true ] || [ "$build_windows" = true ]; } && [ "$host_os" = "Darwin" ]; then
  echo "The Linux/Windows builds have to be run in Docker or WSL, not on a Mac" >&2
  exit 1
fi

rm -rf relbuild

preview_flag=""
if [ "${PREVIEW_BUILD:-false}" = "true" ]; then
  preview_flag="-DPREVIEW_BUILD=ON"
fi

if [ "$build_windows" = true ]; then
  echo ----- Windows -----
  mkdir -p relbuild/windows
  cmake -DCMAKE_TOOLCHAIN_FILE=mingw-cross-toolchain.cmake -B relbuild/windows -S . ${preview_flag}
  cmake --build relbuild/windows -j
  mv relbuild/windows/YummyLife_windows.exe relbuild/
  rm -rf relbuild/windows
fi

if [ "$build_linux" = true ]; then
  echo ----- Linux -----
  mkdir -p relbuild/linux
  cmake -B relbuild/linux -S . ${preview_flag}
  cmake --build relbuild/linux -j
  mv relbuild/linux/YummyLife_linux relbuild/
  rm -rf relbuild/linux
fi

if [ "$build_macos" = true ]; then
  echo ----- macOS -----
  mkdir -p relbuild/macos
  cmake -B relbuild/macos -S . ${preview_flag}
  cmake --build relbuild/macos -j
  mv relbuild/macos/YummyLife.app relbuild/
  rm -rf relbuild/macos
  # A .app is a directory, so it has to be zipped to be a release asset. ditto
  # keeps the bundle's symlinks and signature intact, plain zip does not.
  echo Zipping YummyLife.app
  ditto -c -k --sequesterRsrc --keepParent relbuild/YummyLife.app relbuild/YummyLife_mac.zip
fi

for dst in ahap ohol; do
  if [ -e ~/$dst ]; then
    echo Copying to ~/$dst
    for built in relbuild/YummyLife_linux relbuild/YummyLife_windows.exe relbuild/YummyLife.app; do
      if [ -e "$built" ]; then
        # -R because the mac build is an .app bundle (a directory)
        cp -R "$built" ~/$dst/
      fi
    done
  fi
done
