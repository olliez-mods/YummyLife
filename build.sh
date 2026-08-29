#!/bin/bash
set -e

# YummyLife
#
#
#   ./build.sh <mode> <os>... <target>... ["run"]
#
#   mode      dev or rel/release
#   OS        mac/macos, linux, windows - one or more
#   target    game, editor, server - one or more
#   run       optional, launches everything this run built
#
# Every target builds for every OS, so the OSes and targets you name are built
# as a full cross product: 'linux windows game server' is four binaries.
#
# What you cannot do is build for an OS this host cannot compile for. macOS
# binaries only build on a Mac, because no container or VM can produce them,
# and the Linux and Windows builds are the ones Docker and WSL provide - the
# Windows one by cross-compiling with mingw, since Windows is never the host.
#
# Note for Docker: You may need to have this run twice, first time it runs it might
# fail because of missing CMakeCache.txt, but it will generate it for the second run.

host_os=$(uname -s)

usage() {
  echo "usage: ./build.sh <dev|rel> <mac|linux|windows ...> <game|editor|server ...> [run]" >&2
  echo >&2
  echo "  mode    dev, rel (or release)          exactly one, required" >&2
  echo "  OS      mac (or macos), linux, windows one or more, required" >&2
  echo "  target  game, editor, server           one or more, required" >&2
  echo "  run     launch everything just built   optional" >&2
  echo >&2
  echo "  e.g.  ./build.sh dev mac editor run" >&2
  echo "        ./build.sh rel linux windows game server" >&2
}

mode=""

want_mac=false
want_linux=false
want_windows=false

want_game=false
want_editor=false
want_server=false

do_run=false

for arg in "$@"; do
  case "$arg" in
    dev)             mode=dev ;;
    rel|release)     mode=release ;;

    mac|macos)       want_mac=true ;;
    linux)           want_linux=true ;;
    windows)         want_windows=true ;;

    game)            want_game=true ;;
    editor)          want_editor=true ;;
    server)          want_server=true ;;

    run)             do_run=true ;;
    *)
      echo "Unknown argument '$arg'" >&2
      echo >&2
      usage
      exit 1
      ;;
  esac
done

os_list=()
if [ "$want_mac" = true ];     then os_list+=(mac); fi
if [ "$want_linux" = true ];   then os_list+=(linux); fi
if [ "$want_windows" = true ]; then os_list+=(windows); fi

target_list=()
if [ "$want_game" = true ];   then target_list+=(game); fi
if [ "$want_editor" = true ]; then target_list+=(editor); fi
if [ "$want_server" = true ]; then target_list+=(server); fi

# All three are required
missing=()
if [ -z "$mode" ];                then missing+=("a mode (dev or rel)"); fi
if [ ${#os_list[@]} -eq 0 ];      then missing+=("an OS (mac, linux or windows)"); fi
if [ ${#target_list[@]} -eq 0 ];  then missing+=("a target (game, editor or server)"); fi

if [ ${#missing[@]} -gt 0 ]; then
  for item in "${missing[@]}"; do
    echo "Missing $item" >&2
  done
  echo >&2
  usage
  exit 1
fi

# What this host can compile for
if [ "$want_mac" = true ] && [ "$host_os" != "Darwin" ]; then
  echo "The macOS build has to run natively on a Mac, Docker/WSL cannot produce it" >&2
  exit 1
fi
if { [ "$want_linux" = true ] || [ "$want_windows" = true ]; } && [ "$host_os" = "Darwin" ]; then
  echo "The Linux and Windows builds have to run in Docker or WSL, not on a Mac" >&2
  exit 1
fi

# Everything that differs between a dev and a release build is one of these.
if [ "$mode" = release ]; then
  outdir=relbuild
  cmake_flags=""
  if [ "${PREVIEW_BUILD:-false}" = "true" ]; then
    cmake_flags="-DPREVIEW_BUILD=ON"
  fi
  name_game_mac=YummyLife.app          # the plain name players see
  name_game_linux=YummyLife_linux
  name_game_windows=YummyLife_windows.exe
else
  outdir=devbuild
  cmake_flags="-DTEST_BUILD=ON"
  name_game_mac=YummyLife_dev_mac.app
  name_game_linux=YummyLife_dev_linux
  name_game_windows=YummyLife_dev_windows.exe
fi

# The whole build matrix, in one table. For an OS and a target it gives the
# CMake target to build, what CMake calls the file it produces, what we rename
# that to in $outdir, and - for a .app bundle - the binary inside it, which is
# empty for everything that is already a bare executable
resolve() {
  case "$1:$2" in
    mac:game)        cmake_target=YummyLife_mac;       cmake_output=YummyLife.app;           final_name=$name_game_mac;            bundle_exe=YummyLife ;;
    linux:game)      cmake_target=YummyLife_linux;     cmake_output=YummyLife_linux;         final_name=$name_game_linux;          bundle_exe="" ;;
    windows:game)    cmake_target=YummyLife_windows;   cmake_output=YummyLife_windows.exe;   final_name=$name_game_windows;        bundle_exe="" ;;

    mac:editor)      cmake_target=EditOneLife_mac;     cmake_output=EditOneLife.app;         final_name=EditOneLife.app;           bundle_exe=EditOneLife ;;
    linux:editor)    cmake_target=EditOneLife_linux;   cmake_output=EditOneLife_linux;       final_name=EditOneLife;               bundle_exe="" ;;
    windows:editor)  cmake_target=EditOneLife_windows; cmake_output=EditOneLife_windows.exe; final_name=EditOneLife.exe;           bundle_exe="" ;;

    mac:server)      cmake_target=OneLifeServer;       cmake_output=OneLifeServer;           final_name=OneLifeServer_macos;       bundle_exe="" ;;
    linux:server)    cmake_target=OneLifeServer;       cmake_output=OneLifeServer;           final_name=OneLifeServer_linux;       bundle_exe="" ;;
    windows:server)  cmake_target=OneLifeServer;       cmake_output=OneLifeServer.exe;       final_name=OneLifeServer_windows.exe; bundle_exe="" ;;
  esac
}

# Where each OS configures CMake. Windows gets its own folder because it is the
# one build that does not use the host's compiler.
build_dir_for() {
  case "$1" in
    mac)     build_dir="$outdir/macos";   toolchain_flag="" ;;
    linux)   build_dir="$outdir/linux";   toolchain_flag="" ;;
    windows) build_dir="$outdir/windows"; toolchain_flag="-DCMAKE_TOOLCHAIN_FILE=mingw-cross-toolchain.cmake" ;;
  esac
}

# What this run will produce, under $outdir, and which of those this host can
# then launch. Worked out up front so that the cleanup below removes exactly
# what is about to be rebuilt.
artifacts=()
run_names=()
run_exes=()

for os in "${os_list[@]}"; do
  for target in "${target_list[@]}"; do
    resolve "$os" "$target"
    artifacts+=("$final_name")

    # Nothing cross-compiled for Windows can be launched here; everything else
    # was built by this host's own compiler, so it can
    if [ "$os" != windows ]; then
      run_names+=("$final_name")
      run_exes+=("$bundle_exe")
    fi
  done
done

# Remove old builds. A release build starts from nothing every time; a dev build
# keeps the CMake folders, and with them the CMakeCache, so that it can build
# incrementally.
if [ "$mode" = release ]; then
  rm -rf "$outdir"
else
  for artifact in "${artifacts[@]}"; do
    rm -rf "${outdir:?}/${artifact:?}"
  done
fi

# One CMake configure per OS, then each target by name. Only the game targets
# are in ALL, so naming every target is what keeps a server build from also
# building the client.
for os in "${os_list[@]}"; do
  build_dir_for "$os"
  mkdir -p "$build_dir"
  cmake $toolchain_flag -B "$build_dir" -S . $cmake_flags

  for target in "${target_list[@]}"; do
    resolve "$os" "$target"

    echo "----- $target, for $os -----"
    cmake --build "$build_dir" --target $cmake_target -j

    # Replace rather than move onto: the mac builds are .app bundles, and
    # moving onto an existing directory puts the new one inside it
    rm -rf "${outdir:?}/${final_name:?}"
    mv "$build_dir/$cmake_output" "$outdir/$final_name"

    if [ "$os" = mac ] && [ "$target" = game ] && [ "$mode" = release ]; then
      # A .app is a directory, so it has to be zipped to be a release asset.
      # ditto keeps the bundle's symlinks and signature intact, plain zip does not.
      echo Zipping "$final_name"
      ditto -c -k --sequesterRsrc --keepParent "$outdir/$final_name" "$outdir/YummyLife_mac.zip"
    fi
  done
done

# Once, at the end: an OS folder is shared by every target built out of it, so
# clearing it inside the loop would make the next target reconfigure it.
if [ "$mode" = release ]; then
  rm -rf "$outdir/windows" "$outdir/linux" "$outdir/macos"
fi

# For development, copy to ~/ahap and ~/ohol if the folder exists, for quick
# testing. Only what this run built goes across, so a server build does not
# also push a game that an earlier run happened to leave in $outdir.
copied=false
for dst in ahap ohol; do
  if [ -e ~/$dst ]; then
    copied=true
    echo Copying to ~/$dst
    for artifact in "${artifacts[@]}"; do
      # Replace rather than copy over, for the same reason as above
      rm -rf ~/$dst/"${artifact:?}"
      # -R because the mac builds are bundles, which are directories
      cp -R "$outdir/$artifact" ~/$dst/
    done
    run_dir=~/$dst
  fi
done

# Say where the build went either way. Without this the copy step is silent
# when neither folder is there, which reads like it copied and looks the same
# as the folder having gone missing.
if [ "$copied" = false ]; then
  echo "Built into $outdir/ (neither ~/ahap nor ~/ohol exists, so nothing was copied)"
fi

if [ "$do_run" = true ]; then
  if [ ${#run_names[@]} -eq 0 ]; then
    echo "Nothing to run: nothing cross-compiled for Windows can be launched here" >&2
    exit 1
  fi
  if [ -z "${run_dir:-}" ]; then
    echo "Nothing to run from: symlink a game folder to ~/ohol (or ~/ahap) first, e.g." >&2
    echo "  ln -s /Applications/OneLife ~/ohol" >&2
    exit 1
  fi

  cd "$run_dir"

  # All of them at once, so that 'game server run' brings up both. They stay in
  # this script's process group, so a Ctrl-C at the terminal reaches every one;
  # the trap is for the script being killed some other way.
  pids=()
  trap 'kill "${pids[@]}" 2>/dev/null' INT TERM

  for i in "${!run_names[@]}"; do
    run_name="${run_names[$i]}"
    run_exe="${run_exes[$i]}"

    if [ -n "$run_exe" ]; then
      run_path="$run_dir/$run_name/Contents/MacOS/$run_exe"
    else
      run_path="$run_dir/$run_name"
    fi

    echo ----- running "$run_name" -----
    "$run_path" &
    pids+=($!)
  done

  wait
fi
