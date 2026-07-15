#!/usr/bin/env bash
# alcyon_link.sh -- archive + link every csrc/build/alcyon/*.o into
# LCP.PRG using Alcyon's ar68 + doindex + aln, all driven headless
# via Hatari.
#
# Design notes:
#   - ar68 with more than one .o per command tail routinely triggers
#     COMMAND.PRG's batch-abort bus error, same pattern as the compile
#     phase.  We add ONE .o per hatari launch, ~5 sec each.  For 52
#     objects that's ~5 min real time.
#   - doindex builds the .ndx sidecar file aln needs to resolve
#     archive members.  Runs once after the archive is complete.
#   - aln takes the full command line (gemstart.o + our objects + the
#     three Alcyon library archives) in a single hatari launch.
#
# Environment (same as alcyon_build.sh):
#   HATARI, TOS_IMG, HC_ROOT

set -euo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
HC_ROOT=${HC_ROOT:-$HOME/hatari-c}
HATARI=${HATARI:-hatari}
TOS_IMG=${TOS_IMG:-/opt/homebrew/Cellar/hatari/2.6.1/Hatari.app/Contents/Resources/tos.img}
VBLS=${VBLS:-8000}
OBJDIR=$CSRC/build/alcyon
LOG=$OBJDIR/alcyon_link.log

run_hatari() {
    "$HATARI" --harddrive "$HC_ROOT" --tos "$TOS_IMG" \
        --fast-forward on --fast-boot on --sound off \
        --frameskips 5 --monitor mono --tos-res high \
        --conout 2 --run-vbls "$VBLS" -w --borders off \
        >> "$LOG" 2>&1 || true
}

# Stage the Alcyon libraries into \SRC so the link command line stays
# within TOS's 127-byte command-tail limit (short paths beat long).
stage_libs() {
    cp "$HC_ROOT/TOOLS/LIB/gemstart.o" "$HC_ROOT/SRC/"
    cp "$HC_ROOT/TOOLS/LIB/gem.a"      "$HC_ROOT/SRC/"
    cp "$HC_ROOT/TOOLS/LIB/gem.ndx"    "$HC_ROOT/SRC/"
    cp "$HC_ROOT/TOOLS/LIB/libc.a"     "$HC_ROOT/SRC/"
    cp "$HC_ROOT/TOOLS/LIB/libc.ndx"   "$HC_ROOT/SRC/"
    cp "$HC_ROOT/TOOLS/LIB/libf"       "$HC_ROOT/SRC/"
    # doindex libf too if there's no .ndx sidecar for it
    if [ ! -f "$HC_ROOT/SRC/libf.ndx" ]; then
        {
            echo "echo doindex libf"
            printf '\\TOOLS\\DOINDEX.TTP \\SRC\\libf\n'
        } > "$HC_ROOT/AUTOEXEC.BAT"
        run_hatari
    fi
}

# 1. Stage .o files into workspace.  All filenames are already 8.3-safe.
rm -f "$HC_ROOT/SRC"/*.o "$HC_ROOT/SRC/lcp.a" "$HC_ROOT/SRC/lcp.ndx"
n=0
for o in "$OBJDIR"/*.o; do
    cp "$o" "$HC_ROOT/SRC/"
    n=$((n + 1))
done
echo "$n .o files staged for archive"

: > "$LOG"

# 2. Build lcp.a incrementally, one .o per hatari launch.
echo "==> ar68 phase (one launch per .o)..." | tee -a "$LOG"
added=0
for oo in "$HC_ROOT/SRC"/*.o; do
    stem=$(basename "$oo" .o)
    {
        echo "echo ar68 $stem"
        printf '\\TOOLS\\AR68.TTP -r \\SRC\\lcp.a \\SRC\\%s.o\n' "$stem"
    } > "$HC_ROOT/AUTOEXEC.BAT"
    sz_before=$(stat -f%z "$HC_ROOT/SRC/lcp.a" 2>/dev/null || echo "0")
    run_hatari
    sz_after=$(stat -f%z "$HC_ROOT/SRC/lcp.a" 2>/dev/null || echo "0")
    if [ "$sz_after" -gt "$sz_before" ]; then
        added=$((added + 1))
    else
        echo "  WARN: $stem.o not added to archive (size stayed $sz_before)"
    fi
done
echo "$added / $n .o added to lcp.a" | tee -a "$LOG"

# 3. Index the archive.
echo "==> doindex..." | tee -a "$LOG"
{
    echo "echo doindex lcp.a"
    printf '\\TOOLS\\DOINDEX.TTP \\SRC\\lcp.a\n'
} > "$HC_ROOT/AUTOEXEC.BAT"
run_hatari
[ -f "$HC_ROOT/SRC/lcp.ndx" ] || { echo "ERROR: lcp.ndx not created"; exit 1; }
echo "  lcp.ndx: $(stat -f%z $HC_ROOT/SRC/lcp.ndx) bytes"

# 4. Link.  Stage libraries into \SRC first (shorter paths keep the
# aln command line under TOS's 127-byte tail limit).
#
# CRITICAL: gemstart.o MUST be the first object file so its `__start`
# entry sits at the PRG's text+0 -- otherwise TOS jumps directly into
# whatever function aln placed first (typically our main.o's first
# defined function), bypassing all CRT init and stack setup.  Symptom
# of getting this wrong: PRG loads but hangs with the busy-bee cursor
# because it's spinning inside endless_game_loop() before any init
# has happened.
stage_libs
echo "==> aln (link)..." | tee -a "$LOG"
{
    echo "echo aln"
    printf '\\TOOLS\\ALN.TTP -o \\SRC\\lcp.prg \\SRC\\gemstart.o \\SRC\\main.o \\SRC\\lcp.a \\SRC\\gem.a \\SRC\\libc.a \\SRC\\libf\n'
} > "$HC_ROOT/AUTOEXEC.BAT"
VBLS=30000 run_hatari

if [ -f "$HC_ROOT/SRC/lcp.prg" ]; then
    cp "$HC_ROOT/SRC/lcp.prg" "$OBJDIR/LCP.PRG"
    echo "SUCCESS: $OBJDIR/LCP.PRG ($(stat -f%z $OBJDIR/LCP.PRG) bytes)"
else
    echo "FAILED: no LCP.PRG produced"
    echo "Unresolved symbols from log:"
    grep -A50 "UNRESOLVED" "$LOG" | head -60
    exit 1
fi
