#!/usr/bin/env bash
# trace_lcyof.sh -- log every write to g_lcyof (carrying-object flag).
#
# When g_lcyof gets stuck at YES the game visually freezes: gameTick's
# Path B fires for every subsequent tick and returns without rendering,
# simulating, or reading input.  This script arms a Hatari memory-write
# breakpoint over g_lcyof's 2 bytes with :trace so gameplay isn't
# interrupted; every write dumps the writer's PC + a compact register
# snapshot + the new value being stored.
#
# Play through the "hungry -> cabinet -> freeze" scenario.  When it
# freezes press 'q' to quit Hatari and share the tail of
# /tmp/lcp_lcyof_trace.log with me -- the last few entries before
# the freeze tell us which function left the flag at YES.
#
# Prereqs: source/build/alcyon/LCP.PRG (any config; production build
# recommended: no SKIP_TITLE / SKIP_MIDI / TEST_* flags).
#
# Env:
#   TOS_IMG    default TOS104US.ROM under Retro/
#   GAME_DIR   default ~/hatari-c/GAME
#   PRG        default source/build/alcyon/LCP.PRG
#
# Usage:
#   source/tools/trace_lcyof.sh
#
# Read the tail:
#   tail -50 /tmp/lcp_lcyof_trace.log

set -uo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
TOS_IMG=${TOS_IMG:-/Users/sarnau/Desktop/Retro/Atari ST/Atari TOS Images/TOS104US.ROM}
GAME_DIR=${GAME_DIR:-$HOME/hatari-c/GAME}
PRG=${PRG:-$CSRC/build/alcyon/LCP.PRG}
LOG=/tmp/lcp_lcyof_trace.log

[ -f "$PRG" ]     || { echo "SETUP: LCP.PRG missing at $PRG" >&2; exit 2; }
[ -f "$TOS_IMG" ] || { echo "SETUP: TOS ROM missing at $TOS_IMG" >&2; exit 2; }

# Derive g_lcyof's runtime address from the current build.  BASE for
# --auto-loaded LCP.PRG is fixed by TOS Pexec at 0x12596 (verified via
# `info basepage` in earlier diagnostics).
LCYOF_OFF=$(python3 "$CSRC/tools/find_syms.py" _g_lcyof \
    | awk '$1=="_g_lcyof"{sub(/^base\+/,"",$3); print $3}')
if [ -z "$LCYOF_OFF" ]; then
    echo "SETUP: couldn't derive g_lcyof offset from find_syms" >&2
    exit 2
fi
BASE=0x12596
LCYOF_ADDR=$(printf '%x' $((BASE + LCYOF_OFF)))
# 2-byte range (g_lcyof is a short).
LCYOF_END=$(printf '%x' $((BASE + LCYOF_OFF + 1)))

echo "==== g_lcyof memwatch armed ===="
echo "  address:  \$$LCYOF_ADDR..\$$LCYOF_END (2 bytes)"
echo "  log file: $LOG"
echo ""
echo "Play until the freeze fires, then press 'q' to quit Hatari."
echo "Share the tail of $LOG."
echo ""

WORKDIR=$(mktemp -d)
trap "rm -rf $WORKDIR" EXIT

# On each write, dump just enough to identify the writer: the last 3
# PCs (which include the instruction that just executed the store)
# and the post-store word at g_lcyof.  Kept small so the log stays
# manageable across a 40-min session.
cat > "$WORKDIR/watch.dbg" <<EOF
history 2000
EOF

# Access(range,w) fires on any write into the range.  :trace continues
# emulation after the debugger command runs (unlike a stop-on-hit
# breakpoint).
# Hatari's value-change watch: with `!` and the same expression on
# both sides, the debugger substitutes the current value on the RHS
# and fires whenever the memory word actually changes.
# `history cpu` enables PC recording so the file-hook can dump the
# writer's PC via `history 3`.
cat > "$WORKDIR/run.ini" <<EOF
history cpu 4096
b (\$$LCYOF_ADDR).w ! (\$$LCYOF_ADDR).w :trace :file $WORKDIR/watch.dbg
c
EOF

cp -f "$PRG" "$GAME_DIR/LCP.PRG"
rm -f "$GAME_DIR/LCP.SAV" "$GAME_DIR/HYBER"
pkill -x hatari 2>/dev/null; sleep 1

# --fast-forward runs ~2000 VBLs/sec vs 60 real-time.  30-40 min of
# game time = ~150000 VBLs.  Cap at 250000 for safety margin (~2 min
# host wall-clock).
VBLS=${VBLS:-250000}

hatari \
    --harddrive "$GAME_DIR" \
    --tos "$TOS_IMG" \
    --fast-forward on \
    --auto 'C:\LCP.PRG' \
    --run-vbls "$VBLS" \
    --parse "$WORKDIR/run.ini" > "$LOG" 2>&1 &
HPID=$!

# Freeze detector: sample the trace log's size every 60 host seconds.
# Under --fast-forward at ~1700 VBL/s, 60 s = ~100000 VBLs = ~28 min
# of simulated game time.  In healthy runs, g_lcyof toggles often
# enough that the log always grows.  If the log stagnates for 2
# consecutive samples the game has stalled and we kill Hatari to
# preserve the last valid trace entries.
prev_size=0
stagnant=0
while kill -0 $HPID 2>/dev/null; do
    sleep 60
    cur_size=$(wc -c < "$LOG" 2>/dev/null | tr -d ' ')
    cur_size=${cur_size:-0}
    if [ "$cur_size" = "$prev_size" ]; then
        stagnant=$((stagnant + 1))
        echo "[monitor] log stagnant ($stagnant/2) at $cur_size bytes" >&2
        if [ $stagnant -ge 2 ]; then
            echo "[monitor] freeze detected -- killing Hatari" >&2
            kill $HPID 2>/dev/null
            break
        fi
    else
        stagnant=0
        echo "[monitor] log growing: $prev_size -> $cur_size bytes" >&2
    fi
    prev_size=$cur_size
done
wait $HPID 2>/dev/null
