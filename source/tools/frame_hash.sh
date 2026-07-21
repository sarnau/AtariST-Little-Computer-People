#!/usr/bin/env bash
# frame_hash.sh -- deterministic-frame regression check.
#
# Records a fixed-length Hatari run, extracts every Nth AVI frame,
# md5-hashes each into tools/frame_hash.txt, then diffs against a
# checked-in golden file.  Catches silent rendering regressions
# (sprite drawn wrong, wrong background, missing HUD) the crash
# detector misses because the game keeps running.
#
# Usage:
#   frame_hash.sh              -> generate hashes to tools/frame_hash.txt
#                                 and diff against tools/frame_hash.golden
#   frame_hash.sh update       -> overwrite the golden with fresh hashes
#                                 (do this only when you've verified the
#                                  new output is what you intend)
#
# Env vars:
#   VBLS         run length             (default 3000; below the crash)
#   FPS_SAMPLE   FPS to extract at      (default 1  -> 1 frame per sec)
#   FPS_MAX      cap on frame count     (default 60)
#
# Exit:
#   0 hashes match golden
#   1 mismatch (prints diff)
#   2 setup error (build missing, ffmpeg missing, golden missing)

set -uo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
TOOLS="$CSRC/tools"
GOLDEN="$TOOLS/frame_hash.golden"
OUT="$TOOLS/frame_hash.txt"
AVI=/tmp/lcp_frame_hash.avi
FRAMES=/tmp/lcp_frame_hash_frames

VBLS=${VBLS:-2000}     # small default for fast CI; the v_gtext threshold
                       # this used to dodge was resolved by the initBRev fix
FPS_SAMPLE=${FPS_SAMPLE:-1}
FPS_MAX=${FPS_MAX:-40}

MODE=${1:-check}

# Rebuild with SKIP_TITLE=1 so the interactive title screen doesn't
# hang under --fast-forward.  Skip when NO_REBUILD is set (e.g. by a
# CI job that already ran the build step).
if [ -z "${NO_REBUILD:-}" ]; then
    ALCYON_CPPFLAGS="-DSKIP_TITLE=1 -DSKIP_MIDI=1" "$TOOLS/alcyon_build.sh" >/dev/null 2>&1 \
        && "$TOOLS/alcyon_link.sh" >/dev/null 2>&1 \
        || { echo "SETUP: rebuild for tests failed" >&2; exit 2; }
fi

command -v ffmpeg >/dev/null || { echo "SETUP: ffmpeg not installed" >&2; exit 2; }
command -v md5    >/dev/null || command -v md5sum >/dev/null \
    || { echo "SETUP: no md5/md5sum" >&2; exit 2; }
MD5=$(command -v md5sum >/dev/null && echo "md5sum -b" || echo "md5 -q")

# Reuse run_hatari.sh, but with AVI recording.
AVI="$AVI" VBLS="$VBLS" "$TOOLS/run_hatari.sh" >/dev/null
run_status=$?
if [ ! -f "$AVI" ]; then
    echo "SETUP: Hatari didn't produce $AVI" >&2
    exit 2
fi

rm -rf "$FRAMES" && mkdir -p "$FRAMES"
ffmpeg -y -i "$AVI" -vf "fps=$FPS_SAMPLE" -frames:v "$FPS_MAX" \
    "$FRAMES/f%03d.png" -loglevel error >/dev/null 2>&1

# Hash frames in numeric order.
tmp=$(mktemp)
for f in "$FRAMES"/f*.png; do
    [ -f "$f" ] || continue
    h=$($MD5 "$f" | awk '{print $1}')
    printf "%s  %s\n" "$h" "$(basename "$f")" >> "$tmp"
done
mv "$tmp" "$OUT"
n=$(wc -l < "$OUT" | tr -d ' ')
echo "hashed $n frames -> $OUT"
echo "  run_hatari exit=$run_status (0=clean, 1=crashed)"

if [ "$MODE" = "update" ]; then
    cp "$OUT" "$GOLDEN"
    echo "golden updated at $GOLDEN"
    exit 0
fi

if [ ! -f "$GOLDEN" ]; then
    echo "" >&2
    echo "No golden file at $GOLDEN yet." >&2
    echo "First run: $0 update" >&2
    exit 2
fi

if diff -u "$GOLDEN" "$OUT"; then
    echo "MATCH: $n frames identical to golden"
    exit 0
fi
echo ""
echo "MISMATCH: frame hashes differ from golden"
echo "  investigate the diff above; if the new output is intentional,"
echo "  re-record: $0 update"
exit 1
