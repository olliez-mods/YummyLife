#!/bin/bash
set -e

# Prints the markdown body for a GitHub release to stdout.
#
#   ./yummyLifeBuildScripts/makeReleaseNotes.sh v438.1 artifacts owner/repo [test]
#
# Asset sizes are read off the files in <artifact-dir>, so the tables always
# describe what is actually being uploaded. Any asset that is missing is dropped
# from its table rather than left as a dead link.

TAG=$1
ART_DIR=$2
REPO=$3
TEST=${4:-false}

if [ -z "$TAG" ] || [ -z "$ART_DIR" ] || [ -z "$REPO" ]; then
  echo "Usage: $0 <tag> <artifact-dir> <owner/repo> [test]" >&2
  exit 1
fi

# stat and numfmt differ between GNU and BSD, so do it with wc and awk instead --
# this script runs on the Linux runner but is worth being able to run on a Mac too
humanSize() {
  wc -c < "$1" | awk '{
    n = $1; split("B KB MB GB", u, " "); i = 1
    while( n >= 1024 && i < 4 ) { n /= 1024; i++ }
    printf( (i == 1) ? "%d %s\n" : "%.0f %s\n", n, u[i] )
  }'
}

# Emits one table row, or nothing at all if that asset was not built
row() {
  # row <label> <asset-name> <note>
  label=$1; name=$2; note=$3
  path=$(find "$ART_DIR" -type f -name "$name" -print | head -1)
  [ -n "$path" ] || return 0
  printf '| %s | [%s](https://github.com/%s/releases/download/%s/%s) | %s | %s |\n' \
    "$label" "$name" "$REPO" "$TAG" "$name" "$(humanSize "$path")" "$note"
}

if [ "$TEST" = true ]; then
  cat <<'EOT'
# ⚠️ THIS IS A TEST RELEASE

A mid-version build put up for testing, not a finished release — **it may be
unstable**. The window title says PREVIEW BUILD so you can tell it apart from a
normal one.

If something is broken, please report it. To go back, grab the newest normal
release from [the Releases page](https://github.com/olliez-mods/YummyLife/releases).

---

EOT
fi

# A stub to fill in on the draft before publishing
cat <<'EOT'
# Release!

Notes:
- note 1
- note 2

---

### Drop-in — install over an existing OHOL/AHAP folder

Put the file in the same folder as the vanilla `OneLife.exe` and run it from there.

| Platform | Download | Size | Notes |
|---|---|---|---|
EOT

row "Windows" "YummyLife_windows.exe" "run it in place"
row "Linux"   "YummyLife_linux"       "\`chmod +x\` then run"
row "macOS"   "YummyLife_mac.zip"     "unzip to get \`YummyLife.app\` — see note below"

cat <<'EOT'

---

### OHOL — complete download, nothing else needed

| Platform | Download | Size | Notes |
|---|---|---|---|
EOT

row "Windows" "YummyLife_OHOL_windows.zip" "run \`YummyLife_windows.exe\`"
row "Linux"   "YummyLife_OHOL_linux.zip"   "run \`./YummyLife_linux\`"
row "macOS"   "YummyLife_OHOL_mac.zip"     "keep the \`.app\` in the folder — see note below"

cat <<'EOT'

---

### AHAP — complete download, nothing else needed

| Platform | Download | Size | Notes |
|---|---|---|---|
EOT

row "Windows" "YummyLife_AHAP_windows.zip" "run \`YummyLife_windows.exe\`"
row "Linux"   "YummyLife_AHAP_linux.zip"   "run \`./YummyLife_linux\`"
row "macOS"   "YummyLife_AHAP_mac.zip"     "keep the \`.app\` in the folder — see note below"

cat <<'EOT'

---

**macOS first run.** The app is not notarized, so macOS refuses to open it until you
clear the quarantine flag:

```
xattr -dr com.apple.quarantine YummyLife.app
```
EOT
