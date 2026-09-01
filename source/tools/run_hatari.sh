#!/usr/bin/env bash
# run_hatari.sh -- headless Hatari smoke test with crash detection.
#
# Boots LCP.PRG in Hatari, runs for N VBLs, greps the log for bus
# errors and program-exit lines, prints a concise verdict, and exits
# non-zero on any crash.  Same pattern I've been running by hand all
# session -- lifted into a script so CI / /loop can call it.
#
# Env vars (all optional):
#   VBLS       run length in VBLs        (default 5000)
#   TOS_IMG    TOS ROM to boot           (default TOS104US.ROM under Retro/)
#   GAME_DIR   Hatari GEMDOS-HDD root    (default ~/Hatari_C/hatari-c/GAME)
#   PRG        LCP.PRG to install        (default source/build/alcyon/LCP.PRG)
#   HATARI     hatari binary             (default `hatari` in PATH)
#   AVI        set to a path to record   (default off)
#   LOG        Hatari stderr log target  (default /tmp/lcp_run.log)
#
# Exit codes:
#   0  clean run, no bus errors
#   1  bus error(s) detected
#   2  Hatari didn't launch / PRG missing / setup error

set -uo pipefail

VBLS=${VBLS:-5000}
TOS_IMG=${TOS_IMG:-/Users/sarnau/Desktop/Retro/Atari ST/Atari TOS Images/TOS104US.ROM}
GAME_DIR=${GAME_DIR:-$HOME/Hatari_C/hatari-c/GAME}
PRG=${PRG:-$(cd "$(dirname "$0")/.." && pwd)/build/alcyon/LCP.PRG}
HATARI=${HATARI:-hatari}
AVI=${AVI:-}
LOG=${LOG:-/tmp/lcp_run.log}

if [ ! -f "$PRG" ]; then
    echo "SETUP: LCP.PRG not found at $PRG (build first: make alcyon)" >&2
    exit 2
fi
if [ ! -f "$TOS_IMG" ]; then
    echo "SETUP: TOS image not found at $TOS_IMG" >&2
    exit 2
fi
if [ ! -d "$GAME_DIR" ]; then
    echo "SETUP: GEMDOS HDD dir not found at $GAME_DIR" >&2
    exit 2
fi

cp -f "$PRG" "$GAME_DIR/LCP.PRG"
# Fresh save so cs_mvIn path is exercised.
rm -f "$GAME_DIR/LCP.SAV"

pkill -x hatari 2>/dev/null; sleep 1
rm -f "$LOG"
if [ -n "$AVI" ]; then
    rm -f "$AVI"
    "$HATARI" \
        --harddrive "$GAME_DIR" \
        --tos "$TOS_IMG" \
        --fast-forward on \
        --run-vbls "$VBLS" \
        --auto 'C:\LCP.PRG' \
        --avi-vcodec bmp --avi-file "$AVI" --avirecord \
        > "$LOG" 2>&1
else
    "$HATARI" \
        --harddrive "$GAME_DIR" \
        --tos "$TOS_IMG" \
        --fast-forward on \
        --run-vbls "$VBLS" \
        --auto 'C:\LCP.PRG' \
        > "$LOG" 2>&1
fi

# Filter out $fc0174 -- TOS 1.04's boot-time RAM-size probe deliberately
# triggers a bus error at that PC to detect the top of RAM; it's not a
# real crash and fires on every boot regardless of the loaded program.
n_be=$(grep 'Bus Error' "$LOG" 2>/dev/null | grep -v fc0174 | wc -l | tr -d ' ')
n_be=${n_be:-0}
first_pc=$(grep 'Bus Error' "$LOG" | grep -v fc0174 | head -1 | sed 's/.*PC=\$\([0-9a-f]*\).*/\1/')
uniq_pcs=$(grep 'Bus Error' "$LOG" | grep -v fc0174 | sed 's/.*PC=\$\([0-9a-f]*\).*/\1/' | sort -u | tr '\n' ' ')
exited=$(grep -o 'program 0x[0-9a-f]* exit' "$LOG" | head -1)

echo "==== HATARI SMOKE TEST ===="
echo "VBLs run:    $VBLS"
echo "bus errors:  $n_be"
echo "first PC:    \$${first_pc:-none}"
echo "unique PCs:  ${uniq_pcs:-none}"
echo "exit line:   ${exited:-none}"

if [ "$n_be" -gt 0 ]; then
    echo "VERDICT:     CRASHED"
    exit 1
fi
echo "VERDICT:     clean"
exit 0
