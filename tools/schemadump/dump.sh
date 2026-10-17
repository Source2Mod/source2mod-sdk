#!/usr/bin/env bash
# Source2Mod SDK -- dump the CS2 schema (and convars, commands, entity classes)
# from the dedicated server's binaries.
#
#   tools/schemadump/dump.sh <work dir> <output json>
#
# DumpSource2 loads the game's modules without starting the game and writes
# what they register: schemas.json has the schema classes and enums of every
# module, the convars and concommands and the entity classes. It is built from
# our fork, Source2Mod/DumpSource2, so a CS2 update can be dumped before the
# upstream one (ValveResourceFormat/DumpSource2) has caught up: fix the
# signatures there, the next run picks them up. DUMPSOURCE2_REPO / _REF pick
# another one; DUMPSOURCE2 may point to a built DumpSource2-CS2 instead.
#
# Downloads the Linux binaries (depot 2347773) and the gameinfo files (depot
# 2347770), about 1 GB, no game content.
#
# Needs cmake, ninja, a C++20 compiler, curl, python3, and the desktop
# libraries the client module links against (Ubuntu: libva2 libvdpau1
# libharfbuzz0b libthai0). DEPOTDOWNLOADER may point to a DepotDownloader
# binary; otherwise the latest Linux release is fetched.
set -euo pipefail

[ $# -eq 2 ] || { echo "usage: $0 <work dir> <output json>" >&2; exit 1; }

WORK=$(realpath -m "$1")
OUT=$(realpath -m "$2")
SERVER="$WORK/server"
GAME="$SERVER/game"

APP=730
DEPOT_BINARIES=2347773
DEPOT_CONTENT=2347770
DUMPSOURCE2_REPO=${DUMPSOURCE2_REPO:-https://github.com/Source2Mod/DumpSource2.git}
DUMPSOURCE2_REF=${DUMPSOURCE2_REF:-master}

mkdir -p "$WORK" "$SERVER"

# --- DepotDownloader ---------------------------------------------------------
if [ -z "${DEPOTDOWNLOADER:-}" ]; then
    echo "=== Fetching DepotDownloader"
    url=$(curl -fsSL https://api.github.com/repos/SteamRE/DepotDownloader/releases/latest \
        | python3 -c 'import json,sys; print(next(a["browser_download_url"] for a in json.load(sys.stdin)["assets"] if a["name"] == "DepotDownloader-linux-x64.zip"))')
    curl -fsSL -o "$WORK/dd.zip" "$url"
    python3 -m zipfile -e "$WORK/dd.zip" "$WORK/depotdownloader"
    DEPOTDOWNLOADER="$WORK/depotdownloader/DepotDownloader"
    chmod +x "$DEPOTDOWNLOADER"
fi

# depot_download <depot> <filelist lines...>
depot_download() {
    local depot=$1; shift
    printf '%s\n' "$@" > "$WORK/filelist_$depot.txt"
    "$DEPOTDOWNLOADER" -app "$APP" -depot "$depot" -filelist "$WORK/filelist_$depot.txt" \
        -dir "$SERVER" -max-downloads 8 > "$WORK/depotdownloader_$depot.log" \
        || { tail -20 "$WORK/depotdownloader_$depot.log"; exit 1; }
}

# --- Server files ------------------------------------------------------------
echo "=== Downloading the Linux binaries (depot $DEPOT_BINARIES)"
depot_download "$DEPOT_BINARIES" \
    'regex:^game/bin/linuxsteamrt64/.*' \
    'regex:^game/csgo/bin/linuxsteamrt64/.*' \
    'regex:^game/core/bin/linuxsteamrt64/.*'

echo "=== Downloading the gameinfo files (depot $DEPOT_CONTENT)"
depot_download "$DEPOT_CONTENT" \
    'regex:^game/(csgo|core|csgo_core|csgo_imported)/gameinfo(_branchspecific)?\.gi$' \
    'game/csgo/steam.inf'

# --- DumpSource2 -------------------------------------------------------------
if [ -z "${DUMPSOURCE2:-}" ]; then
    echo "=== Building DumpSource2 ($DUMPSOURCE2_REPO $DUMPSOURCE2_REF)"
    rm -rf "$WORK/DumpSource2"
    git clone -q --recursive --depth 1 --shallow-submodules --branch "$DUMPSOURCE2_REF" \
        "$DUMPSOURCE2_REPO" "$WORK/DumpSource2"
    git -C "$WORK/DumpSource2" log -1 --format='DumpSource2: %h %s'
    cmake -S "$WORK/DumpSource2" -B "$WORK/DumpSource2/build" -G Ninja -DCMAKE_BUILD_TYPE=Release > /dev/null
    cmake --build "$WORK/DumpSource2/build" --target DumpSource2-CS2 > "$WORK/dumpsource2-build.log" \
        || { tail -40 "$WORK/dumpsource2-build.log"; exit 1; }
    DUMPSOURCE2="$WORK/DumpSource2/build/DumpSource2-CS2"
fi

# --- Dump --------------------------------------------------------------------
echo "=== Dumping"
rm -rf "$WORK/dump" && mkdir -p "$WORK/dump"
(
    cd "$GAME/bin/linuxsteamrt64"
    LD_LIBRARY_PATH="$PWD:$GAME/csgo/bin/linuxsteamrt64" "$DUMPSOURCE2" "$WORK/dump" \
        > "$WORK/dumpsource2.log" 2>&1
) || { echo "DumpSource2 failed:" >&2; grep -E '\[(critical|error)\]' "$WORK/dumpsource2.log" >&2 || tail -40 "$WORK/dumpsource2.log" >&2; exit 1; }

python3 - "$WORK/dump/schemas.json" "$GAME/csgo/steam.inf" <<'PY'
import json, sys
d = json.load(open(sys.argv[1]))
patch = next((l.split("=", 1)[1].strip() for l in open(sys.argv[2]) if l.startswith("PatchVersion=")), "?")
server = sum(1 for c in d["classes"] if c["module"] == "server")
print(f"CS2 {patch} (build {d['revision']}): {len(d['classes'])} classes ({server} server), "
      f"{len(d['enums'])} enums, {len(d['convars'])} convars, {len(d['commands'])} commands, {len(d['entities'])} entity classes")
if server < 100:
    sys.exit("The dump has almost no server classes, something went wrong")
PY
mkdir -p "$(dirname "$OUT")"
cp "$WORK/dump/schemas.json" "$OUT"
echo "Wrote $OUT"
