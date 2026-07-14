#!/usr/bin/env bash
# alcyon_build.sh -- drive Alcyon C 4.14 headless via Hatari to build
# every csrc/*.c into a .o under csrc/build/alcyon/.
#
# Design notes accumulated from painful trial-and-error:
#
#  - c068 (the C parser) crashes on complex files EVEN AS IT WRITES
#    ITS OUTPUT.  The crash is fatal for COMMAND.PRG's batch runner
#    but the .1 / .2 files are already on disk when it happens.
#    -> we launch hatari ONCE PER FILE so a crash on file N doesn't
#       poison file N+1.  Adds ~2 sec/file boot overhead but is
#       robust.
#
#  - Long .c filenames (>8 chars) collide at TOS's 8.3 layer.  We
#    stage each source under its short alias from
#    tools/filename_map.txt.  The .o is copied back under the
#    original long name.
#
#  - Long C identifiers hit the 7-char external-symbol truncation
#    inside the compiler.  Handled in-source (see namemap.md).
#
# Environment:
#   HATARI     hatari binary   (default: hatari)
#   TOS_IMG    TOS/EmuTOS ROM
#   HC_ROOT    workspace dir   (default: $HOME/hatari-c)
#   VBLS       hatari --run-vbls per file (default: 15000)
#   FILES      space-separated .c basenames to build (default: all)

set -euo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
HC_ROOT=${HC_ROOT:-$HOME/hatari-c}
HATARI=${HATARI:-hatari}
TOS_IMG=${TOS_IMG:-/opt/homebrew/Cellar/hatari/2.6.1/Hatari.app/Contents/Resources/tos.img}
VBLS=${VBLS:-15000}
OUT=$CSRC/build/alcyon
MAP=$CSRC/tools/filename_map.txt

[ -d "$HC_ROOT/TOOLS" ] || { echo "ERROR: $HC_ROOT/TOOLS missing (bootstrap needed)"; exit 1; }
[ -f "$MAP" ]           || { echo "ERROR: $MAP missing"; exit 1; }

# Load alias map.
# Alias lookup helper: no bash-4 associative arrays on macOS.
lookup_alias() {
    awk -F'\t' -v want="$1" '$1 == want { print $2; found=1; exit }
                              END { if (!found) print want }' "$MAP"
}

# Stage headers once.
rm -rf "$HC_ROOT/SRC"
mkdir -p "$HC_ROOT/SRC" "$OUT"
cp "$CSRC/include"/*.h "$HC_ROOT/SRC/"

# Which files to build?  Env var FILES overrides default (=all).
if [ -n "${FILES:-}" ]; then
    TO_BUILD="$FILES"
else
    TO_BUILD=""
    for c in "$CSRC"/*.c; do
        base=$(basename "$c")
        [ "$base" = "save_host.c" ] && continue
        TO_BUILD="$TO_BUILD $base"
    done
fi

built=0; missed=0; total=0
missed_list=""
for base in $TO_BUILD; do
    total=$((total + 1))
    src="$CSRC/$base"
    [ -f "$src" ] || { echo "  SKIP: $base (not found)"; missed=$((missed + 1)); continue; }
    alias_base="$(lookup_alias "$base")"
    stem="${alias_base%.c}"

    # Stage this file
    cp "$src" "$HC_ROOT/SRC/$alias_base"

    # Phase 1: cp68 + c068.  c068 may crash on complex files but it
    # writes its .1/.2 output BEFORE crashing, so we recover in phase 2.
    {
        echo "echo $base phase1"
        printf '\\TOOLS\\CP68.TTP -P -D__ALCYON__ -I\\TOOLS\\INCLUDE -I\\SRC \\SRC\\%s.c \\SRC\\%s.i\n' "$stem" "$stem"
        printf '\\TOOLS\\C068.TTP \\SRC\\%s.i \\SRC\\%s.1 \\SRC\\%s.2 \\SRC\\%s.3 -f\n' "$stem" "$stem" "$stem" "$stem"
    } > "$HC_ROOT/AUTOEXEC.BAT"
    "$HATARI" --harddrive "$HC_ROOT" --tos "$TOS_IMG" \
        --fast-forward on --fast-boot on --sound off \
        --frameskips 5 --monitor mono --tos-res high \
        --conout 2 --run-vbls "$VBLS" -w --borders off \
        >> "$OUT/alcyon_build.log" 2>&1 || true

    # Phase 2a: c168 in a fresh COMMAND.PRG (may crash but writes .s
    # before crashing).
    if [ -f "$HC_ROOT/SRC/$stem.1" ] && [ -f "$HC_ROOT/SRC/$stem.2" ]; then
        {
            echo "echo $base phase2a"
            printf '\\TOOLS\\C168.TTP \\SRC\\%s.1 \\SRC\\%s.2 \\SRC\\%s.s\n' "$stem" "$stem" "$stem"
        } > "$HC_ROOT/AUTOEXEC.BAT"
        "$HATARI" --harddrive "$HC_ROOT" --tos "$TOS_IMG" \
            --fast-forward on --fast-boot on --sound off \
            --frameskips 5 --monitor mono --tos-res high \
            --conout 2 --run-vbls "$VBLS" -w --borders off \
            >> "$OUT/alcyon_build.log" 2>&1 || true
    fi

    # Post-process: dedup L-labels that c168 emits at each new .text
    # boundary.  as68 refuses to assemble a file with duplicate labels;
    # rename second-and-later occurrences to L<n>_<k>.
    if [ -f "$HC_ROOT/SRC/$stem.s" ]; then
        python3 -c "
import re, sys
p = '$HC_ROOT/SRC/${stem}.s'
lines = open(p).readlines()
seen = {}
out = []
for ln in lines:
    m = re.match(r'^(L\d+):(.*)\$', ln)
    if m:
        lbl = m.group(1)
        seen[lbl] = seen.get(lbl, 0) + 1
        if seen[lbl] > 1:
            out.append(f'{lbl}_{seen[lbl]}:{m.group(2)}\n')
            continue
    out.append(ln)
open(p, 'w').writelines(out)
"
    fi

    # Phase 2b: as68 in a fresh COMMAND.PRG.
    if [ -f "$HC_ROOT/SRC/$stem.s" ]; then
        {
            echo "echo $base phase2b"
            printf '\\TOOLS\\AS68.TTP -l -u \\SRC\\%s.s\n' "$stem"
        } > "$HC_ROOT/AUTOEXEC.BAT"
        "$HATARI" --harddrive "$HC_ROOT" --tos "$TOS_IMG" \
            --fast-forward on --fast-boot on --sound off \
            --frameskips 5 --monitor mono --tos-res high \
            --conout 2 --run-vbls "$VBLS" -w --borders off \
            >> "$OUT/alcyon_build.log" 2>&1 || true
    fi

    if [ -f "$HC_ROOT/SRC/$stem.o" ]; then
        cp "$HC_ROOT/SRC/$stem.o" "$OUT/${base%.c}.o"
        built=$((built + 1))
        printf '  [%2d/%2d] OK   %s\n' "$total" "$(echo $TO_BUILD | wc -w | tr -d ' ')" "$base"
    else
        missed=$((missed + 1))
        missed_list="$missed_list $base"
        printf '  [%2d/%2d] MISS %s\n' "$total" "$(echo $TO_BUILD | wc -w | tr -d ' ')" "$base"
    fi
done

echo
echo "Alcyon build: $built OK, $missed missing.  Log: $OUT/alcyon_build.log"
if [ -n "$missed_list" ]; then
    echo "Missing:$missed_list"
fi
exit $([ "$missed" -eq 0 ] && echo 0 || echo 1)
