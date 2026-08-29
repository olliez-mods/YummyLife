#!/bin/bash
set -e

# Builds a complete, ready-to-play game folder for one platform and one game
# (OHOL or AHAP) and zips it.
#
#   ./yummyLifeBuildScripts/makeReleaseFolder.sh v438.1 windows ohol
#
# This does NOT compile anything -- run './build.sh release <target>' first and
# this packages what it left in relbuild/.
#
# The game data is not in this repo, it comes from Jason's data repos, which the
# release workflow checks out into dataRepos/ (see .github/workflows/release.yml).

# This script expects to be run from the repo root or from yummyLifeBuildScripts/
cd "$(dirname "$0")/.."

if [ $# -lt 3 ]; then
  echo "Usage: $0 <version> <platform> <game>"
  echo "  version:   release tag, e.g. v438.1"
  echo "  platform:  linux | windows | mac"
  echo "  game:      ohol | ahap"
  exit 1
fi

VERSION=$1
PLATFORM=$2
GAME=$3

case "$GAME" in
  ohol)
    DATA_DIR=dataRepos/OneLifeData7
    DATA_REPO=jasonrohrer/OneLifeData7
    # Bump when a newer one is out -- this is only the example in the hint below
    DATA_TAG=OneLife_v437
    ;;
  ahap)
    DATA_DIR=dataRepos/AnotherPlanetData
    DATA_REPO=jasonrohrer/AnotherPlanetData
    DATA_TAG=AnotherPlanet_v81
    ;;
  *) echo "$0: unknown game '$GAME' (expected ohol or ahap)" >&2; exit 1 ;;
esac

case "$PLATFORM" in
  linux)   BUILT=relbuild/YummyLife_linux       ; BUILD_CMD="./build.sh release linux game"   ;;
  windows) BUILT=relbuild/YummyLife_windows.exe ; BUILD_CMD="./build.sh release windows game" ;;
  mac)     BUILT=relbuild/YummyLife.app         ; BUILD_CMD="./build.sh release mac game"     ;;
  *) echo "$0: unknown platform '$PLATFORM' (expected linux, windows or mac)" >&2; exit 1 ;;
esac

# Printed whenever the data checkout is missing or unusable
dataCheckoutHint() {
  echo "  git clone --depth 1 --branch $DATA_TAG https://github.com/$DATA_REPO $DATA_DIR" >&2
}

if [ ! -d "$DATA_DIR" ]; then
  echo "$0: no $GAME game data at $DATA_DIR" >&2
  dataCheckoutHint
  exit 1
fi

if [ ! -f "$DATA_DIR/dataVersionNumber.txt" ]; then
  echo "$0: $DATA_DIR has no dataVersionNumber.txt, incomplete checkout" >&2
  dataCheckoutHint
  exit 1
fi

DATA_VERSION=$(tr -d '[:space:]' < "$DATA_DIR/dataVersionNumber.txt")
if [ -z "$DATA_VERSION" ]; then
  echo "$0: $DATA_DIR/dataVersionNumber.txt is empty" >&2
  dataCheckoutHint
  exit 1
fi

# Up front, so a missing build fails immediately rather than after several
# minutes of copying game data into a folder that can never be finished
if [ ! -e "$BUILT" ]; then
  echo "$0: no $PLATFORM build at $BUILT -- this script packages, it does not compile" >&2
  echo "  $BUILD_CMD" >&2
  exit 1
fi

if [ "$PLATFORM" = windows ] && [ ! -f SDL-1.2.15/bin/SDL.dll ]; then
  # Linux/Docker only: on macOS setup-libs.sh gets SDL from Homebrew, no DLL
  echo "$0: SDL-1.2.15/bin/SDL.dll is missing, a standalone folder must ship one" >&2
  echo "  cd yummyLifeBuildScripts && ./setup-libs.sh" >&2
  exit 1
fi

# The folder players see once they unzip. Both numbers are in it because the mod
# version and the game data version move independently -- the same scheme Jason
# uses in scripts/bundleAHAPDataIntoWindowsAndLinuxBinaryBuilds.sh
FOLDER_NAME=YummyLife_${VERSION}_d${DATA_VERSION}
OUT_ROOT=relbuild/release
FOLDER=$OUT_ROOT/$FOLDER_NAME

GAME_UPPER=$(echo "$GAME" | tr '[:lower:]' '[:upper:]')
ZIP_NAME=YummyLife_${GAME_UPPER}_${PLATFORM}.zip

mkdir -p "$OUT_ROOT"
rm -rf "$FOLDER"


##### Assets that ship with every folder, from this repo

mkdir -p "$FOLDER"/{graphics,otherSounds,settings,languages}
# Empty on purpose, see the note about caches at the top
mkdir -p "$FOLDER"/{groundTileCache,reverbCache}

cp gameSource/graphics/*.tga      "$FOLDER/graphics/"
cp gameSource/otherSounds/*.aiff  "$FOLDER/otherSounds/"
cp gameSource/settings/*.ini      "$FOLDER/settings/"
cp gameSource/languages/*.txt     "$FOLDER/languages/"

cp gameSource/language.txt \
   gameSource/us_english_60.txt \
   gameSource/wordList.txt \
   gameSource/liveTriggers.txt \
   gameSource/reverbImpulseResponse.aiff \
   gameSource/eqImpulseResponse.aiff \
   "$FOLDER/"

cp documentation/Readme.txt "$FOLDER/"
cp no_copyright.txt         "$FOLDER/"


##### Game data, from the data repo

for d in sprites objects categories transitions animations music sounds ground; do
  if [ ! -d "$DATA_DIR/$d" ]; then
    echo "$0: $DATA_DIR/$d is missing, so the data checkout is incomplete" >&2
    dataCheckoutHint
    exit 1
  fi
  mkdir -p "$FOLDER/$d"
  # src/. rather than src/* so this does not blow the argument limit on
  # transitions/, which is tens of thousands of small files
  cp -R "$DATA_DIR/$d/." "$FOLDER/$d/"
done

cp "$DATA_DIR/dataVersionNumber.txt" "$FOLDER/"

# game.cpp reads this at startup to decide which game it is running as
if [ "$GAME" = ahap ]; then
  echo 1 > "$FOLDER/isAHAP.txt"
fi

# Never ship a developer's credentials lol
rm -f "$FOLDER/settings/email.ini" "$FOLDER/settings/accountKey.ini"

find "$FOLDER" -name '*~' -delete

##### Line endings
# Do this before the mac bundle is copied in, so it cannot touch files inside the
# .app and invalidate its signature.

if [ "$PLATFORM" = windows ]; then
  find "$FOLDER" -type f -name '*.txt' -print0 | xargs -0 perl -pi -e 's/\r?\n/\r\n/'
else
  find "$FOLDER" -type f -name '*.txt' -print0 | xargs -0 perl -pi -e 's/\r\n/\n/'
fi


##### Platform payload

case "$PLATFORM" in

  linux)
    cp "$BUILT" "$FOLDER/"
    ;;

  windows)
    cp "$BUILT" "$FOLDER/"
    # The exe links SDL through an import library, so the DLL has to travel with
    # it
    cp SDL-1.2.15/bin/SDL.dll "$FOLDER/"
    ;;

  mac)
    cp -R "$BUILT" "$FOLDER/"
    ;;

esac


##### Archive

rm -f "$OUT_ROOT/$ZIP_NAME"

if [ "$PLATFORM" = mac ]; then
  # ditto, not zip: it preserves the symlinks and code signature inside the .app
  ditto -c -k --sequesterRsrc --keepParent "$FOLDER" "$OUT_ROOT/$ZIP_NAME"
else
  ( cd "$OUT_ROOT" && zip -qr "$ZIP_NAME" "$FOLDER_NAME" )
fi

# The unzipped folder is ~700MB and the runner is tight on disk with six of these
rm -rf "$FOLDER"

echo "Built $OUT_ROOT/$ZIP_NAME"
