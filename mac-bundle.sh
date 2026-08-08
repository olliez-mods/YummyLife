#!/bin/bash
set -e

# Makes a freshly built YummyLife.app self-contained.
#
# The app links against Homebrew libraries (sdl12-compat, OpenSSL), which a
# player will not have installed, so every non-system library it needs gets
# copied into YummyLife.app/Contents/Frameworks and every reference to it
# rewritten to @executable_path/../Frameworks. CMake runs this as a POST_BUILD
# step, it is not meant to be run by hand.
#
# Usage: mac-bundle.sh <path to YummyLife.app> [executable name]

APP="$1"
if [ -z "$APP" ]; then
  echo "usage: $0 <path to .app> [executable name]" >&2
  exit 1
fi

APP_NAME=$(basename "$APP" .app)
EXE_NAME="${2:-$APP_NAME}"
EXE="$APP/Contents/MacOS/$EXE_NAME"
FRAMEWORKS="$APP/Contents/Frameworks"

if [ ! -x "$EXE" ]; then
  echo "no executable at $EXE" >&2
  exit 1
fi

mkdir -p "$FRAMEWORKS"

# A dependency needs bundling unless it ships with macOS itself. Matching on
# the system prefixes rather than on /opt/homebrew keeps this working for both
# Homebrew prefixes (/opt/homebrew on Apple Silicon, /usr/local on Intel) and
# for the Cellar paths OpenSSL records internally.
needs_bundling() {
  case "$1" in
    /usr/lib/*|/System/*) return 1 ;;
    /*)                   return 0 ;;
    *)                    return 1 ;;
  esac
}

# Copy a library into Contents/Frameworks and point its own install name at the
# bundled copy. Returns without doing anything if it is already there.
bundle_library() {
  src="$1"
  name=$(basename "$src")
  if [ -e "$FRAMEWORKS/$name" ]; then
    return 1
  fi
  echo "  bundling $name"
  cp -L "$src" "$FRAMEWORKS/$name"
  chmod u+w "$FRAMEWORKS/$name"
  install_name_tool -id "@executable_path/../Frameworks/$name" "$FRAMEWORKS/$name"
  return 0
}

# Walk the dependency graph breadth-first starting from the executable. Each
# binary we visit can pull in more Homebrew libraries (OpenSSL's libssl brings
# in libcrypto, for instance), so newly bundled libraries go back on the queue.
queue=("$EXE")
while [ ${#queue[@]} -gt 0 ]; do
  current="${queue[0]}"
  queue=("${queue[@]:1}")

  # Skip otool's header line, then take the path from each dependency line.
  for dep in $(otool -L "$current" | tail -n +2 | awk '{print $1}'); do
    if ! needs_bundling "$dep"; then
      continue
    fi

    name=$(basename "$dep")

    # A dylib lists its own install name first, that is not a dependency.
    if [ "$current" = "$FRAMEWORKS/$name" ]; then
      continue
    fi

    if bundle_library "$dep"; then
      queue+=("$FRAMEWORKS/$name")
    fi

    # Always rewrite, the executable is relinked on every incremental build
    # even when Contents/Frameworks survived from the last one.
    install_name_tool -change "$dep" "@executable_path/../Frameworks/$name" "$current"
  done
done

# sdl12-compat does not link SDL2, it dlopen()s it at runtime, and sdl2-compat
# in turn dlopen()s SDL3, so otool cannot see either of them. Both look in
# @loader_path first, which is Contents/Frameworks once sdl12-compat lives
# there, so copying them in beside it is all that is needed.
for pkg_lib in "sdl2-compat:libSDL2-2.0.0.dylib" "sdl3:libSDL3.dylib"; do
  pkg="${pkg_lib%%:*}"
  lib="${pkg_lib##*:}"

  found=""
  prefix=$(brew --prefix "$pkg" 2>/dev/null || true)
  for candidate in "$prefix/lib/$lib" "/opt/homebrew/lib/$lib" "/usr/local/lib/$lib"; do
    if [ -e "$candidate" ]; then
      found="$candidate"
      break
    fi
  done

  if [ -z "$found" ]; then
    echo "$lib not found, install it with: brew install $pkg" >&2
    exit 1
  fi

  bundle_library "$found" || true
done

# install_name_tool invalidates any existing signature, and on Apple Silicon an
# unsigned binary will not run at all, so everything has to be re-signed. A real
# Developer ID identity can be supplied through YUMMYLIFE_CODESIGN, otherwise we
# ad-hoc sign, which is enough to launch locally but still leaves players having
# to clear the quarantine flag on a downloaded build.
if [ -n "$YUMMYLIFE_CODESIGN" ]; then
  sign_args=(--force --sign "$YUMMYLIFE_CODESIGN" --timestamp --options runtime)
  echo "Signing with identity: $YUMMYLIFE_CODESIGN"
else
  sign_args=(--force --sign -)
  echo "YUMMYLIFE_CODESIGN not set, ad-hoc signing"
fi

xattr -cr "$APP"
# Nested code has to be signed before the bundle that contains it.
for lib in "$FRAMEWORKS"/*.dylib; do
  codesign "${sign_args[@]}" "$lib"
done
codesign "${sign_args[@]}" "$APP"
codesign --verify --deep "$APP"

echo "Bundled $(ls "$FRAMEWORKS" | wc -l | tr -d ' ') libraries into $(basename "$APP")"
