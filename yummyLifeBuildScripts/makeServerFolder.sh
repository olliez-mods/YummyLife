#!/bin/bash
set -e

# Builds a complete, ready-to-run server folder for one platform and one game
# (OHOL or AHAP) and zips it.
#
#   ./yummyLifeBuildScripts/makeServerFolder.sh v438.1 linux ohol
#
# This does NOT compile anything -- run './build.sh rel <platform> server' first
# and this packages what it left in relbuild/.

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
  linux)   BUILT=relbuild/OneLifeServer_linux       ;;
  windows) BUILT=relbuild/OneLifeServer_windows.exe ;;
  mac)     BUILT=relbuild/OneLifeServer_macos       ;;
  *) echo "$0: unknown platform '$PLATFORM' (expected linux, windows or mac)" >&2; exit 1 ;;
esac
BUILD_CMD="./build.sh rel $PLATFORM server"

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

# Up front, so a missing build fails immediately rather than after minutes of
# copying game data into a folder that can never be finished
if [ ! -e "$BUILT" ]; then
  echo "$0: no $PLATFORM server at $BUILT -- this script packages, it does not compile" >&2
  echo "  $BUILD_CMD" >&2
  exit 1
fi

FOLDER_NAME=YummyLifeServer_${VERSION}_d${DATA_VERSION}
OUT_ROOT=relbuild/release
FOLDER=$OUT_ROOT/$FOLDER_NAME

GAME_UPPER=$(echo "$GAME" | tr '[:lower:]' '[:upper:]')
ZIP_NAME=YummyLifeServer_${GAME_UPPER}_${PLATFORM}.zip

mkdir -p "$OUT_ROOT"
rm -rf "$FOLDER"
mkdir -p "$FOLDER"


##### Settings, from this repo's server/ folder
#
# As serverSettings/ rather than settings/: server.cpp prefers that folder when
# it exists (setUseServerSettings), and it means this package can be unzipped
# into an existing game folder without trampling the client's settings/.

mkdir -p "$FOLDER/serverSettings"
cp server/settings/*.ini "$FOLDER/serverSettings/"

# Never ship a developer's credentials, same as the client folder
rm -f "$FOLDER/serverSettings/email.ini" "$FOLDER/serverSettings/accountKey.ini"

# Settings a standalone server wants set differently from the values the repo
# carries, which are tuned for the live official servers.
#
#   use*Server        vanilla points these at Jason's central services, which a
#                     private server has no account on and should not call
#   allowVOGMode      shipped on, with vogAllowAccounts naming a live developer
#                     account -- that would be an admin on someone else's server
#   clientPassword    shipped as a placeholder; blank it rather than ship one
#   mapCellForgotten  the live servers forget map cells after a week, which is
#                     wrong for a small server that wants its world to persist
setSetting() {
  # setSetting <name> <value>, value may be empty
  printf '%s' "$2" > "$FOLDER/serverSettings/$1.ini"
}

setSetting useArcServer      0
setSetting useCurseServer    0
setSetting useFitnessServer  0
setSetting useLifeServer     0
setSetting useLineageServer  0
setSetting useStatsServer    0

setSetting allowVOGMode      0
setSetting vogAllowAccounts  ""
setSetting clientPassword    ""

setSetting mapCellForgottenSeconds 999999999999

# port stays at the 8005 the repo ships, and maxPlayers at its default


##### Server data files, from this repo's server/ folder
#
# firstNames.txt is deliberately not here: nothing reads it, names.cpp loads
# only the male, female and last name lists.

cp server/maleNames.txt \
   server/femaleNames.txt \
   server/lastNames.txt \
   server/curseWordList.txt \
   server/wordList.txt \
   "$FOLDER/"

# The client and server refuse each other when their code versions differ, so
# derive this rather than copy the checked-in server/serverCodeVersionNumber.txt,
# which goes stale silently.

# [0-9][0-9]* rather than [0-9]+, which BSD sed on the macOS runner does not take
SERVER_CODE_VERSION=$(sed -n 's/^int versionNumber *= *\([0-9][0-9]*\).*/\1/p' gameSource/game.cpp | head -1)
if [ -z "$SERVER_CODE_VERSION" ]; then
  echo "$0: could not read versionNumber out of gameSource/game.cpp" >&2
  exit 1
fi
echo "$SERVER_CODE_VERSION" > "$FOLDER/serverCodeVersionNumber.txt"


##### Game data, from the data repo

for d in objects categories transitions tutorialMaps contentSettings; do
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

# server.cpp reads this at startup to decide which game it is running
if [ "$GAME" = ahap ]; then
  echo 1 > "$FOLDER/isAHAP.txt"
fi

cp no_copyright.txt "$FOLDER/"

find "$FOLDER" -name '*~' -delete


##### Readme

cat > "$FOLDER/Readme.txt" <<EOF
YummyLife server $VERSION (game data $DATA_VERSION)

Running it
----------
Start the server binary in this folder. It works from anywhere -- run it by
absolute path, from a service file, or by double-clicking; it finds this folder
on its own and runs out of it.

It listens on port 8005 by default. Change that in serverSettings/port.ini.

Settings
--------
Everything is in serverSettings/. The calls out to the central account, life,
lineage, stats, fitness and curse servers are all turned off in this package,
so it runs standalone with no external services.

Created on first run
--------------------
The map and player databases (*.db) and the log folders (lifeLog, curseLog,
foodLog, failureLog and friends) are all created here the first time the server
starts. Back up the .db files to keep a world.

Note that a server folder is not a game folder. Keep this separate from a
YummyLife/OneLife client install rather than unzipping it on top of one.
EOF


##### Line endings

if [ "$PLATFORM" = windows ]; then
  find "$FOLDER" -type f \( -name '*.txt' -o -name '*.ini' \) -print0 | xargs -0 perl -pi -e 's/\r?\n/\r\n/'
else
  find "$FOLDER" -type f \( -name '*.txt' -o -name '*.ini' \) -print0 | xargs -0 perl -pi -e 's/\r\n/\n/'
fi


##### Platform payload
#
# A bare binary on every platform -- no .app bundle and no SDL.dll, because the
# server links no SDL, OpenGL or OpenSSL

cp "$BUILT" "$FOLDER/"


##### Archive

rm -f "$OUT_ROOT/$ZIP_NAME"
( cd "$OUT_ROOT" && zip -qr "$ZIP_NAME" "$FOLDER_NAME" )

echo "$0: wrote $OUT_ROOT/$ZIP_NAME"

# Kept small on purpose, but the runner still has several of these to make
rm -rf "$FOLDER"
