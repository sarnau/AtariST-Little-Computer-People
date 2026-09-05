#!/usr/bin/env bash
# test_longrun_stable.sh -- catch per-tick runaway bugs that only
# surface after minutes of gameplay.
#
# The 2026-07-19 getKey `!= 0` bug (fix in commit f1a5349) fired
# tx_sctm=160 on every game tick and locked the split-copy compositor
# forever.  The visible corruption -- red horizontal stripes over the
# upper 2/3 of the screen -- appeared around VBL 11 500 (~3 real
# minutes of gameplay).  Every existing smoke test in tools/ ran for
# ≤ 5 000 VBLs and never triggered it.  A shape-audit against Ghidra
# had passed; literal-audit had not been performed.
#
# This regression check exists so that this class of bug -- silent
# per-tick corruption that accumulates over minutes -- gets caught in
# seconds by CI.
#
# Method:
#   1. Build with -DSKIP_TITLE=1 -DSKIP_COPYPROT=1.  SKIP_TITLE gets
#      past the interactive guestbook (it waits on getKey for a name,
#      a date and a time, so an unattended run stalls there for ever)
#      and SKIP_COPYPROT past cp_main, which never succeeds under any
#      emulator here and otherwise parks the resident in
#      `while (1) a_sleep(-1);` -- a motionless screen this test would
#      score as a clean PASS.
#      NOT -DSKIP_MIDI: without the Timer-A ISR the mq_* engine never
#      completes a record, the resident retries the activity, and each
#      retry's Fsfirst("*.sng") leaks a TOS folder buffer until GEMDOS
#      halts with "OUT OF INTERNAL MEMORY" -- measured at ~75 s of
#      gameplay.  MIDI jitter does not matter here: this test compares
#      two frames by PSNR, it does not hash them.
#   2. Launch LCP.PRG under `--auto` for 15 000 VBLs.  --auto matches
#      the launch mode where getKey misbehaviour actually surfaces
#      -- COMMAND.PRG (Atari shell) also triggers it, and the whole
#      point is to catch it BEFORE it gets to the user.
#   3. Extract a frame at VBL 3 000 (early, before any corruption
#      could reasonably build up) and a frame at VBL 13 000 (well
#      past the ~11 500 VBL threshold where the getKey bug used to
#      show).
#   4. Compute per-pixel mean-squared-error between the two frames
#      via ffmpeg's psnr filter.  A clean run scores PSNR > 30 dB
#      (the resident moves and the dog wanders, but the house
#      background is static).  A corrupted run drops to < 20 dB
#      because red-stripe overlay covers most of the frame.
#
# Env:
#   VBLS_TOTAL   run length              (default 15000)
#   VBL_EARLY    "known-clean" sample    (default 3000)
#   VBL_LATE     "after-runaway" sample  (default 13000)
#   PSNR_MIN     minimum acceptable dB   (default 25)
#   TOS_IMG      TOS ROM                 (default TOS104US.ROM under Retro/)
#   NO_REBUILD   skip alcyon_build.sh + alcyon_link.sh (for CI reuse)
#
# Exit codes:
#   0  clean (PSNR ≥ threshold, no bus errors)
#   1  corruption detected (PSNR below threshold, or bus error)
#   2  setup error (build missing, ffmpeg missing, Hatari didn't run)

set -uo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
TOOLS="$CSRC/tools"

# 30000, not the 15000 this ran at until 2026-09-06.  The g_sfDoB
# overrun crash (see globals.c) landed at VBL 16983 -- 1983 VBLs past
# the old ceiling, so this test walked right up to it and stopped.  A
# regression check that ends before the bug does is worth nothing;
# 30000 VBLs is ~8 emulated minutes and costs ~20 s of wall clock.
VBLS_TOTAL=${VBLS_TOTAL:-30000}
VBL_EARLY=${VBL_EARLY:-3000}
VBL_LATE=${VBL_LATE:-25000}
PSNR_MIN=${PSNR_MIN:-18}
TOS_IMG=${TOS_IMG:-/Users/sarnau/Desktop/Retro/Atari ST/Atari TOS Images/TOS104US.ROM}
GAME_DIR=${GAME_DIR:-$HOME/Hatari_C/hatari-c/GAME}
PRG=$CSRC/build/alcyon/LCP.PRG

AVI=$(mktemp -t lcp_longrun.XXXXXX).avi
EARLY_PNG=$(mktemp -t lcp_early.XXXXXX).png
LATE_PNG=$(mktemp -t lcp_late.XXXXXX).png
LOG=$(mktemp -t lcp_longrun_log.XXXXXX)
trap 'rm -f "$AVI" "$EARLY_PNG" "$LATE_PNG" "$LOG"' EXIT

# ---- rebuild (unless caller asked us to skip) ---------------------
if [ -z "${NO_REBUILD:-}" ]; then
    ALCYON_CPPFLAGS="-DSKIP_TITLE=1 -DSKIP_COPYPROT=1" \
        "$TOOLS/alcyon_build.sh" >/dev/null 2>&1 \
        && "$TOOLS/alcyon_link.sh" >/dev/null 2>&1 \
        || { echo "SETUP: rebuild failed" >&2; exit 2; }
fi

command -v ffmpeg >/dev/null \
    || { echo "SETUP: ffmpeg missing" >&2; exit 2; }
[ -f "$PRG" ]     || { echo "SETUP: $PRG missing" >&2; exit 2; }
[ -f "$TOS_IMG" ] || { echo "SETUP: TOS ROM missing at $TOS_IMG" >&2; exit 2; }

# ---- run Hatari with --auto + AVI record -------------------------
cp -f "$PRG"                    "$GAME_DIR/LCP.PRG"
rm -f "$GAME_DIR/LCP.SAV"       # fresh cs_mvIn path
pkill -x hatari 2>/dev/null; sleep 1

hatari --harddrive "$GAME_DIR" \
       --tos "$TOS_IMG" \
       --fast-forward on \
       --run-vbls "$VBLS_TOTAL" \
       --auto 'C:\LCP.PRG' \
       --avi-vcodec bmp --avi-file "$AVI" --avirecord on \
       > "$LOG" 2>&1
run_status=$?

if [ ! -s "$AVI" ]; then
    echo "SETUP: Hatari didn't produce an AVI" >&2
    exit 2
fi

# ---- extract the two comparison frames --------------------------
ffmpeg -y -i "$AVI" -vf "select=eq(n\,$VBL_EARLY)" -frames:v 1 \
       -update 1 "$EARLY_PNG" >/dev/null 2>&1
ffmpeg -y -i "$AVI" -vf "select=eq(n\,$VBL_LATE)"  -frames:v 1 \
       -update 1 "$LATE_PNG"  >/dev/null 2>&1

if [ ! -s "$EARLY_PNG" ] || [ ! -s "$LATE_PNG" ]; then
    echo "SETUP: couldn't extract both frames from AVI" >&2
    exit 2
fi

# ---- per-pixel similarity via PSNR ------------------------------
# ffmpeg's psnr filter emits: `PSNR ... average:XX.XX min:...`.
psnr_line=$(ffmpeg -i "$EARLY_PNG" -i "$LATE_PNG" \
                   -filter_complex "psnr" -f null - 2>&1 \
            | grep 'PSNR' | tail -1)
psnr_avg=$(echo "$psnr_line" | sed -E 's/.*average:([0-9.]+).*/\1/' | head -1)

# ---- bus-error check ---------------------------------------------
# Filter out the harmless boot-time warning at $41fffe/PC=$fc0174.
# TOS always writes past the top of the reserved area during its
# ram-size probe; Hatari flags it but it isn't a real crash.
n_be=$(grep 'Bus Error' "$LOG" 2>/dev/null | grep -v fc0174 | wc -l | tr -d ' ')
n_be=${n_be:-0}

# ---- report + verdict -------------------------------------------
echo "==== LONG-RUN STABILITY TEST ===="
echo "  VBLs:        $VBLS_TOTAL"
echo "  early frame: VBL $VBL_EARLY"
echo "  late frame:  VBL $VBL_LATE"
echo "  bus errors:  $n_be"
echo "  PSNR (avg):  ${psnr_avg:-N/A} dB"
echo "  threshold:   ${PSNR_MIN} dB"
echo ""

if [ "$n_be" -gt 0 ]; then
    echo "VERDICT:     CRASHED"
    echo "  ($n_be bus error(s) in Hatari log)"
    exit 1
fi

if [ -z "$psnr_avg" ]; then
    echo "SETUP: couldn't parse PSNR from ffmpeg output" >&2
    echo "raw line: $psnr_line" >&2
    exit 2
fi

# Awk comparison so we don't have to depend on bc.
below=$(awk -v p="$psnr_avg" -v t="$PSNR_MIN" \
        'BEGIN { print (p+0 < t+0) ? 1 : 0 }')
if [ "$below" = "1" ]; then
    echo "VERDICT:     CORRUPTED"
    echo "  early vs late frame PSNR ${psnr_avg} < ${PSNR_MIN} dB;"
    echo "  screen drifted over the run -- a per-tick runaway is"
    echo "  the usual cause.  Check for constant / operator"
    echo "  mistranslations against Ghidra."
    exit 1
fi

echo "VERDICT:     STABLE"
exit 0
