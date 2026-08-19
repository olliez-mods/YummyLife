#!/bin/bash
set -e

# YummyLife

# Made to work with WSL and Docker scripts (it is in CRLF format, so be careful with that)
# Does not clear CMakeCache.txt, so it generally runs faster than build-release.sh

# Note for Docker: You may need to have this run twice, first time it runs it might
# fail because of missing CMakeCache.txt, but it will generate it for the second run.

# Optional params (windows/linux/macos) to only build for specific platforms, pass in
# nothing to build everything this host can produce.
# macOS binaries can only be built on a Mac (Docker/WSL cannot cross-compile them),
# and the Linux/Windows builds are the ones the Docker image provides.
host_os=$(uname -s)

build_linux=false
build_windows=false
build_macos=false
build_editor=false

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
      # The OHOL editor, mac only for now
      editor)     build_editor=true ;;
      *)
        echo "Unknown target '$target' (expected: linux, windows, macos, editor)" >&2
        exit 1
        ;;
    esac
  done
fi

if [ "$build_macos" = true ] && [ "$host_os" != "Darwin" ]; then
  echo "The macOS build has to be run natively on a Mac, Docker/WSL cannot produce it" >&2
  exit 1
fi
if [ "$build_editor" = true ] && [ "$host_os" != "Darwin" ]; then
  echo "The editor build has to be run natively on a Mac, Docker/WSL cannot produce it" >&2
  exit 1
fi
if { [ "$build_linux" = true ] || [ "$build_windows" = true ]; } && [ "$host_os" = "Darwin" ]; then
  echo "The Linux/Windows builds have to be run in Docker or WSL, not on a Mac" >&2
  exit 1
fi

# Remove old builds
rm -rf ./devbuild/YummyLife_*
if [ "$build_editor" = true ]; then rm -rf ./devbuild/EditOneLife.app; fi

if [ "$build_windows" = true ]; then
  echo ----- Windows -----
  mkdir -p devbuild/windows
  cmake -DCMAKE_TOOLCHAIN_FILE=mingw-cross-toolchain.cmake -B devbuild/windows -S . -DTEST_BUILD=ON
  cmake --build devbuild/windows -j
  mv devbuild/windows/YummyLife_windows.exe devbuild/YummyLife_dev_windows.exe
fi

if [ "$build_linux" = true ]; then
  echo ----- Linux -----
  mkdir -p devbuild/linux
  cmake -B devbuild/linux -S . -DTEST_BUILD=ON
  cmake --build devbuild/linux -j
  mv devbuild/linux/YummyLife_linux devbuild/YummyLife_dev_linux
fi

if [ "$build_macos" = true ]; then
  echo ----- macOS -----
  mkdir -p devbuild/macos
  cmake -B devbuild/macos -S . -DTEST_BUILD=ON
  cmake --build devbuild/macos -j
  # The mac build is an .app bundle, not a bare binary, so that it can carry
  # its own copy of SDL and OpenSSL. Drop it in the game folder and run it.
  rm -rf devbuild/YummyLife_dev_mac.app
  mv devbuild/macos/YummyLife.app devbuild/YummyLife_dev_mac.app
fi

if [ "$build_editor" = true ]; then
  echo ----- editor -----
  mkdir -p devbuild/macos
  cmake -B devbuild/macos -S . -DTEST_BUILD=ON
  # EXCLUDE_FROM_ALL in CMakeLists, so the target has to be named
  cmake --build devbuild/macos --target EditOneLife_mac -j
  mv devbuild/macos/EditOneLife.app devbuild/EditOneLife.app
fi

# For development, copy to ~/ahap and ~/ohol if folder exists for quick testing
for dst in ahap ohol; do
  if [ -e ~/$dst ]; then
    echo Copying to ~/$dst
    for built in devbuild/YummyLife_dev_* devbuild/EditOneLife.app; do
      if [ -e "$built" ]; then
        # -R because the mac build is an .app bundle (a directory)
        cp -R "$built" ~/$dst/
      fi
    done
  fi
done
