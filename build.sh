#!/bin/bash
set -e

# YummyLife
#
#   ./build.sh                  dev build of everything this host can produce
#   ./build.sh release          the same, as a release build
#   ./build.sh editor           the OHOL editor
#   ./build.sh release macos    a mode and a target
#   ./build.sh linux windows    dev is implied
#   ./build.sh run              build, then launch it out of the game folder
#
# Modes (dev, release) and targets (linux, windows, macos, editor) can be given
# in any order. macOS binaries can only be built on a Mac (Docker/WSL cannot
# cross-compile them), and the Linux/Windows builds are the ones the Docker
# image provides.
#
# Note for Docker: You may need to have this run twice, first time it runs it might
# fail because of missing CMakeCache.txt, but it will generate it for the second run.

host_os=$(uname -s)

mode=dev
build_linux=false
build_windows=false
build_macos=false
build_editor=false
have_target=false

do_run=false
run_name=""
run_exe=""
run_dir=""

for arg in "$@"; do
  case "$arg" in
    dev|release) mode="$arg" ;;
    linux)       build_linux=true;   have_target=true ;;
    windows)     build_windows=true; have_target=true ;;
    macos|mac)   build_macos=true;   have_target=true ;;
    # The OHOL editor, mac only for now. TEST_BUILD/PREVIEW_BUILD are only read
    # by game.cpp, hetuwmod.cpp and yummyLife.cpp, none of which the editor links
    editor)      build_editor=true;  have_target=true ;;
    run)         do_run=true ;;
    *)
      echo "Unknown argument '$arg' (modes: dev, release; targets: linux, windows, macos, editor; also: run)" >&2
      exit 1
      ;;
  esac
done

# No target named, so build whatever this host can produce
if [ "$have_target" = false ]; then
  if [ "$host_os" = "Darwin" ]; then
    build_macos=true
  else
    build_linux=true
    build_windows=true
  fi
fi

if { [ "$build_macos" = true ] || [ "$build_editor" = true ]; } && [ "$host_os" != "Darwin" ]; then
  echo "The macOS and editor builds have to be run natively on a Mac, Docker/WSL cannot produce them" >&2
  exit 1
fi
if { [ "$build_linux" = true ] || [ "$build_windows" = true ]; } && [ "$host_os" = "Darwin" ]; then
  echo "The Linux/Windows builds have to be run in Docker or WSL, not on a Mac" >&2
  exit 1
fi

# Everything that differs between a dev and a release build is one of these
if [ "$mode" = release ]; then
  outdir=relbuild
  cmake_flags=""
  if [ "${PREVIEW_BUILD:-false}" = "true" ]; then
    cmake_flags="-DPREVIEW_BUILD=ON"
  fi
  name_linux=YummyLife_linux
  name_windows=YummyLife_windows.exe
  # not YummyLife_mac.app, the release bundle is the plain name players see
  name_mac=YummyLife.app
else
  outdir=devbuild
  cmake_flags="-DTEST_BUILD=ON"
  name_linux=YummyLife_dev_linux
  name_windows=YummyLife_dev_windows.exe
  name_mac=YummyLife_dev_mac.app
fi

# Remove old builds. A release build starts from nothing every time; a dev build
# keeps the per-platform folders, and with them the CMakeCache, so that it can
# build incrementally.
if [ "$mode" = release ]; then
  rm -rf "$outdir"
else
  # Only what we are about to rebuild: clearing every YummyLife_* here would
  # make "./build.sh editor" throw away the game app built by a previous run.
  if [ "$build_linux" = true ];   then rm -rf "./$outdir/$name_linux"; fi
  if [ "$build_windows" = true ]; then rm -rf "./$outdir/$name_windows"; fi
  if [ "$build_macos" = true ];   then rm -rf "./$outdir/$name_mac"; fi
  if [ "$build_editor" = true ];  then rm -rf "./$outdir/EditOneLife.app"; fi
fi

if [ "$build_windows" = true ]; then
  echo ----- Windows -----
  mkdir -p "$outdir/windows"
  cmake -DCMAKE_TOOLCHAIN_FILE=mingw-cross-toolchain.cmake -B "$outdir/windows" -S . $cmake_flags
  cmake --build "$outdir/windows" -j
  mv "$outdir/windows/YummyLife_windows.exe" "$outdir/$name_windows"
fi

if [ "$build_linux" = true ]; then
  echo ----- Linux -----
  mkdir -p "$outdir/linux"
  cmake -B "$outdir/linux" -S . $cmake_flags
  cmake --build "$outdir/linux" -j
  mv "$outdir/linux/YummyLife_linux" "$outdir/$name_linux"
  run_name="$name_linux"
fi

if [ "$build_macos" = true ]; then
  echo ----- macOS -----
  mkdir -p "$outdir/macos"
  cmake -B "$outdir/macos" -S . $cmake_flags
  cmake --build "$outdir/macos" -j
  # The mac build is an .app bundle, not a bare binary, so that it can carry
  # its own copy of SDL and OpenSSL. Drop it in the game folder and run it.
  rm -rf "$outdir/$name_mac"
  mv "$outdir/macos/YummyLife.app" "$outdir/$name_mac"
  run_name="$name_mac"
  run_exe=YummyLife

  if [ "$mode" = release ]; then
    # A .app is a directory, so it has to be zipped to be a release asset. ditto
    # keeps the bundle's symlinks and signature intact, plain zip does not.
    echo Zipping "$name_mac"
    ditto -c -k --sequesterRsrc --keepParent "$outdir/$name_mac" "$outdir/YummyLife_mac.zip"
  fi
fi

if [ "$build_editor" = true ]; then
  echo ----- editor -----
  mkdir -p "$outdir/macos"
  cmake -B "$outdir/macos" -S . $cmake_flags
  # EXCLUDE_FROM_ALL in CMakeLists, so the target has to be named
  cmake --build "$outdir/macos" --target EditOneLife_mac -j
  rm -rf "$outdir/EditOneLife.app"
  mv "$outdir/macos/EditOneLife.app" "$outdir/EditOneLife.app"
  run_name=EditOneLife.app
  run_exe=EditOneLife
fi

# Once, at the end: the macos folder is shared by the macos and editor targets,
# so clearing it inside either block would make the other reconfigure it.
if [ "$mode" = release ]; then
  rm -rf "$outdir/windows" "$outdir/linux" "$outdir/macos"
fi

# For development, copy to ~/ahap and ~/ohol if folder exists for quick testing
for dst in ahap ohol; do
  if [ -e ~/$dst ]; then
    echo Copying to ~/$dst
    for built in "$outdir/$name_linux" "$outdir/$name_windows" "$outdir/$name_mac" "$outdir/EditOneLife.app"; do
      if [ -e "$built" ]; then
        # -R because the mac build is an .app bundle (a directory)
        cp -R "$built" ~/$dst/
      fi
    done
    run_dir=~/$dst
  fi
done

if [ "$do_run" = true ]; then
  if [ -z "$run_name" ]; then
    echo "Nothing to run: no runnable target was built (the Windows build cannot be launched here)" >&2
    exit 1
  fi
  if [ -z "$run_dir" ]; then
    echo "Nothing to run from: symlink a game folder to ~/ohol (or ~/ahap) first, e.g." >&2
    echo "  ln -s /Applications/OneLife ~/ohol" >&2
    exit 1
  fi

  if [ -n "$run_exe" ]; then
    run_path="$run_dir/$run_name/Contents/MacOS/$run_exe"
  else
    run_path="$run_dir/$run_name"
  fi

  echo ----- running "$run_name" -----
  cd "$run_dir"
  exec "$run_path"
fi
