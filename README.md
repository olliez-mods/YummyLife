# What is YummyLife?

YummyLife is a mod for OHOL and AHAP based on [hetuw](https://github.com/hetuw/OneLife).
The goals of this mod are to stay up to date with the latest changes to the vanilla
OHOL and AHAP client, fix bugs, and occasionally add useful features.
YummyLife (by OliverZ) was originally forked from YumLife (By Selb) to reintroduce Phex support and more.

## Contents

- **[Installing (and updating)](#installing-and-updating)** — [Steam](#steam-users) · [direct download](#direct-download-users) · [no game installed?](#no-game-installed)
- **[Usage](#usage)**
- **[Troubleshooting](#troubleshooting)** — [update the base game](#make-sure-the-base-game-is-updated) · [clear cache files](#clear-cache-files) · [reinstall](#reinstall) · [still stuck](#still-not-working)
- **[Building from source](#building-from-source)** — [which platform builds what](#which-platform-builds-what) · [setup](#setup) · [Docker](#building-with-docker) · [macOS](#building-on-macos) · [the editor](#the-ohol-editor) · [the server](#the-game-server)
- **[Making a release](#making-a-release)**
- **[Merging upstream changes](#merging-upstream-changes)**

# Installing (and updating)

## Steam users:

1. Make sure the game is fully updated in Steam.
2. Run the game from Steam once to ensure the Steam login details are properly set up.
3. Download the latest version of the mod from [the Releases page](https://github.com/olliez-mods/YummyLife/releases). For Windows this is YummyLife_windows.exe, for Linux YummyLife_linux, and for macOS YummyLife_mac.zip (unzip it to get YummyLife.app).
4. Install the mod into the OHOL/AHAP installation folder (Steam users: right click game > Manage > Browse local files)
5. Run the mod from the OHOL/AHAP installation folder.

## Direct download users:

1. Download the latest version of the mod from [the Releases page](https://github.com/olliez-mods/YummyLife/releases). For Windows this is YummyLife_windows.exe, for Linux YummyLife_linux, and for macOS YummyLife_mac.zip (unzip it to get YummyLife.app).
2. Install the mod into the OHOL/AHAP installation folder (same folder as the vanilla `OneLife.exe`).
3. Run the mod from the OHOL/AHAP installation folder.

## No game installed?

Each release also carries **full downloads** — a complete, ready-to-play folder with
the game data already in it, one per platform for OHOL and for AHAP. Unzip it anywhere
and run it; nothing else is needed.

These are much larger than the drop-in builds, and they ship without an account key, so
you will be asked to log in on first launch. **If you own the game on Steam, use the
drop-in build above instead** — installing into the Steam folder is what lets the mod
pick up your Steam login.

macOS note: the app is not notarized, so the first launch needs a right-click > Open
instead of a double-click (or `xattr -dr com.apple.quarantine YummyLife.app`).

# Usage

Press `H` in-game to see everything the mod can do. A `yummylife.cfg` file is
generated in the OHOL/AHAP install folder and can be tweaked to your liking.

# Troubleshooting

## Make sure the base game is updated

If using Steam, launch Steam and make sure there isn't a pending update on the
base game.

Non-Steam installs will generally update themselves properly, but older installs
may have subtly corrupt data files due to the way YumLife/hetuw updating used to
work. See Reinstall below.

## Clear cache files

Search the OHOL/AHAP installation folder (Steam users: right click game > Manage >
Browse local files) for any `.fcz` files and delete all of them. They will be
regenerated automatically the next time you launch the game.

## Reinstall

Especially with non-Steam installs, there are a variety of issues past and present
that can subtly corrupt data files and leave your game directory in a state that isn't
salvageable. Uninstalling, reinstalling, and then following the
"Installation" section again carefully is the closest thing available to a 100%
certain fix.

The non-Steam version can be re-downloaded from: `http://onehouronelife.com/ticketServer/server.php?action=show_downloads&ticket_id=YOURKEYHERE`
(Note the 'YOURKEYHERE' in the url)

## Still not working?

Open a bug report using the Issues tab above.

# Building from source

Everything is driven by one script, `./build.sh`, on every platform. Every build says
what it is, in full — there are no defaults and nothing is inferred. Name a mode, at
least one OS and at least one target, and you get exactly that and nothing else:

| | values | |
| --- | --- | --- |
| mode | `dev`, `rel` (or `release`) | exactly one, required |
| OS | `mac` (or `macos`), `linux`, `windows` | one or more, required |
| target | `game`, `editor`, `server` | one or more, required |
| run | `run` | optional |

```
./build.sh dev mac game run          # build the client and launch it
./build.sh dev mac editor            # just the editor
./build.sh rel linux windows game    # the release clients for both
./build.sh rel windows server        # the server, for Windows
./build.sh dev linux game server run # two targets, launched together
```

The OSes and targets you name are built as a full **cross product**, because all three
targets build for all three platforms — `linux windows game server` is four binaries.
Order does not matter.

| target | what it is |
| --- | --- |
| `game` | the client — `YummyLife.app` on a Mac, a bare binary otherwise |
| `editor` | Jason's object/sprite/animation editor (see below) |
| `server` | the headless game server (see below) |

The one thing you cannot ask for is an OS this host cannot compile for, and the script
says so rather than building something else. macOS binaries only build on a Mac, because
no container or VM can produce them; the Linux and Windows builds are the ones Docker and
WSL provide, the Windows one by cross-compiling with mingw, since Windows is never the
host.

Here is where each one lands:

| | dev build | release build |
| --- | --- | --- |
| `mac game` | `devbuild/YummyLife_dev_mac.app` | `relbuild/YummyLife.app` + `YummyLife_mac.zip` |
| `linux game` | `devbuild/YummyLife_dev_linux` | `relbuild/YummyLife_linux` |
| `windows game` | `devbuild/YummyLife_dev_windows.exe` | `relbuild/YummyLife_windows.exe` |
| `mac editor` | `devbuild/EditOneLife.app` | `relbuild/EditOneLife.app` |
| `linux editor` | `devbuild/EditOneLife` | `relbuild/EditOneLife` |
| `windows editor` | `devbuild/EditOneLife.exe` | `relbuild/EditOneLife.exe` |
| `mac server` | `devbuild/OneLifeServer_macos` | `relbuild/OneLifeServer_macos` |
| `linux server` | `devbuild/OneLifeServer_linux` | `relbuild/OneLifeServer_linux` |
| `windows server` | `devbuild/OneLifeServer_windows.exe` | `relbuild/OneLifeServer_windows.exe` |

A dev build is compiled with `TEST_BUILD`, which marks the window title and enables
test-only features; a release build is what the Releases page carries. Release builds
start clean every time, dev builds reuse the previous one and are much faster. Only the
client is renamed per mode: the editor and the server keep one name in both, so that a
rebuild replaces the copy already sitting in a game folder.

If you symlink your AHAP and/or OHOL directories to `~/ahap` and `~/ohol`, everything a
run builds is copied there automatically — and only what that run built, so building
the server does not also push a client left over from last time. Adding `run` then
launches every copy that this host can execute, all at once, out of that folder; a
Ctrl-C stops the lot. Nothing cross-compiled for Windows can be launched, on either
host.

## Which platform builds what

All three targets — game, editor and server — exist for all three platforms. What
decides which you can build is the host, not the target: Windows and Linux binaries are
both built on Linux, the Windows ones by cross-compiling with mingw, and macOS binaries
can only be built on a Mac, because no container or VM can produce them.

So:

- **On Windows** — use Docker (below). It builds both the Windows and Linux binaries.
- **On Linux** — build natively, or use Docker. Naming `windows` cross-compiles.
- **On macOS** — build the mac app natively with Homebrew. Docker will also give you
  Linux and Windows binaries, with the caveat noted below.

## Setup

Clone the repo, then do the one-time setup for your platform. Either way this fetches
the vendored libraries (SDL for the Windows cross-build, a prebuilt 32-bit OpenSSL,
`cpp-httplib` and `nlohmann/json`) into the repo root.

### Docker

Needs Docker installed, nothing else. From the `yummyLifeBuildScripts` folder:

```
docker compose run --rm setup
```

(On older Docker installs the command is `docker-compose` with a hyphen.)

### macOS

Install [Homebrew](https://brew.sh/), then from the `yummyLifeBuildScripts` folder:

```
./setup-libs.sh
```

That installs `cmake`, `sdl12-compat`, `openssl@3` and `libpng` through Homebrew and
downloads the header-only libraries. SDL 1.2 itself is unmaintained and no longer builds
on modern macOS; [sdl12-compat](https://github.com/libsdl-org/sdl12-compat) provides the
same API on top of SDL2, and is what Homebrew ships.

### Linux

Install the build dependencies:

```
sudo apt install g++ make cmake libsdl1.2-dev libglu-dev libgl-dev libpng-dev libssl-dev curl
```

Add this if you also want to cross-compile the Windows binary:

```
sudo apt install g++-mingw-w64-i686-win32
```

Then from the `yummyLifeBuildScripts` folder:

```
./setup-libs.sh
```

Package names above are Debian/Ubuntu. `mingw-cross-toolchain.cmake` hardcodes
Debian's `/usr/bin/i686-w64-mingw32-*` paths, so on another distro either edit that file
or use Docker for the Windows build.

## Building with Docker

Run from the `yummyLifeBuildScripts` folder. Anything after `build` goes straight to
`build.sh`, so every command in the reference at the top of this section works here too:

```
docker compose run --rm build dev linux game           # the Linux client
docker compose run --rm build dev windows server      # the Windows server
docker compose run --rm build rel linux windows game  # both release clients
docker compose run --rm shell                  # a shell in the container, to poke around
```

Binaries land in `devbuild/` or `relbuild/` in the repo, same as a native build.

Two things worth knowing:

- The container runs as root, so on a Linux host the build output ends up root-owned.
  To build as yourself instead:
  `DOCKER_USER="$(id -u):$(id -g)" docker compose run --rm build dev linux game`
- On an Apple Silicon Mac, Docker produces **arm64** Linux binaries, not the x86_64 ones
  the Releases page carries. Fine for checking that something compiles, not for shipping.

WSL works too if you would rather not use Docker on Windows — it is an ordinary Linux
environment, so follow the Linux instructions inside it. Native Windows toolchains such
as MSYS2 are not supported: they link against DLLs that the shipped build does not have.

## Building on macOS

```
./build.sh dev mac game   # devbuild/YummyLife_dev_mac.app
./build.sh rel mac game   # relbuild/YummyLife.app + relbuild/YummyLife_mac.zip
```

The result is a self-contained bundle: SDL and OpenSSL are copied into
`Contents/Frameworks` and the icon is generated from `mac_icon.png`, so it runs on a Mac
with no Homebrew installed. The release form also zips it with `ditto`, which preserves
the bundle's symlinks and code signature where a plain `zip` would not.

To sign with a real Developer ID instead of the default ad-hoc signature, set
`YUMMYLIFE_CODESIGN` to the identity name before building.

### Running it

Copy `YummyLife.app` next to the game data folders — the mac OHOL distribution is a
folder holding `animations/`, `objects/`, `sprites/` and so on, alongside
`OneLife_v###.app` — and double-click. The app sets its working directory to the folder
it sits in, so it finds the game data by itself.

Because the release is not notarized, macOS refuses to open it the first time. Either
right-click the app and pick Open, or clear the quarantine flag:

```
xattr -dr com.apple.quarantine YummyLife.app
```

If you don't own the Steam/mac version, the same data folders can be obtained from
[OneLifeData7](https://github.com/jasonrohrer/OneLifeData7) (checkout the latest
`OneLife_v*` tag), plus the `graphics/`, `otherSounds/`, `languages/`, `settings/`,
`language.txt`, `us_english_60.txt`, `wordList.txt` and the two `*.aiff` files from this
repo's `gameSource/`.

## The OHOL editor

Jason's object/sprite/animation editor builds from this tree too, for all three
platforms:

```
./build.sh dev mac editor       # devbuild/EditOneLife.app
./build.sh dev linux editor     # devbuild/EditOneLife
./build.sh dev windows editor   # devbuild/EditOneLife.exe
```

It needs a PNG decoder, which is the one dependency it has over the client — it imports
sprites from PNG files. The native builds use the system libpng (Homebrew's on macOS,
`libpng-dev` on Linux); the Windows build has no system copy to use, so `setup-libs.sh`
fetches the zlib and libpng sources and they are compiled straight into the binary.

It reads its data folders from wherever it sits, so drop it into the game folder next to
`YummyLife.app`. It also needs the editor-only art in `graphics/` (button icons and so
on) — those live in `gameSource/graphics/` and may not be in a player install, so copy
them across if the editor comes up with missing buttons.

See `documentation/EditorAndServerBuildNotes.txt` for what the editor actually does.

## The game server

The headless server (`OneLifeServer`) builds from this tree too, for all three
platforms:

```
./build.sh dev mac server       # devbuild/OneLifeServer_macos
./build.sh dev linux server     # devbuild/OneLifeServer_linux
./build.sh dev windows server   # devbuild/OneLifeServer_windows.exe
```

Unlike the client, the server carries the platform in its name in both modes. Every
platform builds it to a bare binary, so without that suffix a machine that builds both
natively and through Docker would have the two silently overwrite each other.

It links no SDL, OpenGL or OpenSSL, so it needs none of the client's libraries. The
Windows build is a console program rather than a windowed one, so it keeps its stdout,
and it uses the `select()`-based `SocketPollUnix.cpp` — despite the folder that file
lives in, it is not poll-based and carries its own `WIN32` branch. Only Linux gets the
epoll implementation. Winsock's `FD_SETSIZE` of 64 is smaller than the Unix 1024, so a
Windows server makes more `select` calls per wait, which that file already batches for.

Running it takes a little more than building it. The server reads everything relative
to its working directory, so it wants a folder holding `serverSettings/` (copy
`server/settings`) and the game data — `objects`, `categories`, `transitions` and
`tutorialMaps`, which upstream symlinks in from a data checkout. It also wants the
name lists (`maleNames.txt`, `femaleNames.txt`, `lastNames.txt`) and `contentSettings/`.
The scripts in `server/` (`runServerTestMap.sh` and friends) assume they are run from
such a folder with the binary sitting in it.

Unlike upstream, the binary does not have to be launched from that folder. If the
working directory holds no `objects`, the server moves to the directory the executable
itself sits in and runs from there, printing which folder it picked — so dropping
the server binary into a prepared server folder and running it by absolute path works
from anywhere. A working directory that *does* have data always wins, which keeps the
usual "run from the server folder" setup unchanged. If neither has data, the server
now exits with an error instead of starting up empty; an empty object bank used to
leave it serving `displayID = -1`, which segfaults every client that connects.

Note that a server folder is not a game folder: point it at your `YummyLife.app`
folder and its `settings/` and `.db` files will collide with the client's.

# Making a release

Releases are built by `.github/workflows/release.yml`. Pushing to `master` runs it,
and it derives the release tag straight from `gameSource/game.cpp`:

```c
int versionNumber = 438;
const char *yumSubVersion = ".1";   ->   v438.1
```

If a release with that tag already exists, the run stops there. So bumping either of
those two constants is what triggers a release — nothing else to fill in.

The build then produces nine assets: the three drop-in binaries, plus a full
ready-to-play folder for OHOL and for AHAP on each platform. The full folders are
assembled by `yummyLifeBuildScripts/makeReleaseFolder.sh`, which takes the binaries
`./build.sh rel <os> game` left in `relbuild/` and combines them with this repo's
`gameSource` assets and the game data from Jason's data repos:

```
./yummyLifeBuildScripts/makeReleaseFolder.sh v438.1 windows ohol
```

The data is not vendored here — the workflow checks it out into `dataRepos/` at the
latest `OneLife_v*` / `AnotherPlanet_v*` tag. To run the script by hand, clone it there
yourself first (shallow, at a tag — the tree alone is ~550MB):

```
git clone --depth 1 --branch OneLife_v437 \
  https://github.com/jasonrohrer/OneLifeData7 dataRepos/OneLifeData7
```

The script prints that command for the right repo if the data is missing. An OHOL
folder and an AHAP folder differ only by their data and by an `isAHAP.txt` file, which
`game.cpp` reads at startup.

The release notes are generated by `yummyLifeBuildScripts/makeReleaseNotes.sh`, which
builds the download tables from the assets actually uploaded, so the sizes and links
are always real. It leaves a stub at the top to fill in before publishing:

```
# Release!

Notes:
- note 1
- note 2
```

**Test builds.** Running the workflow manually with `test_release` puts up a
mid-version build for people to try. It bumps nothing: the tag is the current version
plus the next free `-test-N`, so `v438.1-test-1`, `v438.1-test-2` and so on against the
same version. It is compiled with `PREVIEW_BUILD`, which marks the window title, the
notes carry a "THIS IS A TEST RELEASE" banner, and it is marked as a GitHub pre-release.

# Merging upstream changes

First, set up remotes for Jason's OneLife and minorGems repos. This only needs to be
done once. Note that upstream OHOL is two repos, which YummyLife condenses into one for
easier forking.

```
$ git remote add OneLife git@github.com:jasonrohrer/OneLife.git
$ git remote add minorGems git@github.com:jasonrohrer/minorGems.git
```

To merge in changes from the OneLife repo, do a `git pull OneLife master` and resolve any
merge conflicts carefully.

Similarly, the minorGems repo can be merged with `git pull minorGems master`. Note that you
will need to move (as in `git mv`) any _new_ files added to that repo into the `minorGems`
directory.

Since YummyLife uses CMake instead of Jason's build scripts, manual updates are needed when
upstream source files are added, removed, or moved. The source lists are the only part that
changes for that, and they live beside the build rather than in it:

| file | what it holds |
| --- | --- |
| `cmake/MinorGemsSources.cmake` | which parts of `minorGems/` each program compiles |
| `cmake/GameSources.cmake` | the client, editor and server source lists |
| `cmake/MacBundle.cmake` | the `.app` icon, plist and signing machinery |
| `CMakeLists.txt` | compile options, the per-platform branch, and the targets |

`cmake/GameSources.cmake` mirrors upstream's `makeFileList`, `makeFileListEditor` and
`server/makeFileList`, and each list says so in a comment, so the two can be checked against
each other after a merge.