#!/usr/bin/env bash
# test_stairs.sh -- verify the resident can descend from the top floor
# (attic) to the bottom floor (kitchen/living room) via both staircases
# without sliding through a floor.
#
# Method:
#   1. Boot LCP.PRG; let cs_mvIn's move-in cutscene complete (~1500 VBLs).
#   2. Force lcp_x/lcp_y to the top-floor staircase entry (attic center
#      just above the upper flight), clear walk waypoint, set walk target
#      to the bottom-floor centre.
#   3. Run for a further N VBLs to let lcp_wkD drive the descent.
#   4. Sample lcp_x/lcp_y/lcp_st/lcp_stR at several checkpoints and
#      verify:
#        - resident actually crossed each floor boundary
#        - lcp_stR was YES while crossing (i.e. descended via stairs)
#        - lcp_st took values in the stair-state ranges (9..24)
#        - resident arrived on the bottom floor (getFlrY(lcp_y) == 1)
#
# Env:
#   VBLS       total run length     (default 6000)
#   TOS_IMG    TOS ROM to boot      (see run_hatari.sh defaults)
#
# Exit codes:
#   0  descent verified
#   1  descent failed (resident stuck, wrong floor, no stair state, etc.)
#   2  setup error / bp never fired
#
# The address list below is derived from `python3 tools/extract_syms.py`
# on the current build; when the memory layout shifts (data-segment
# grow/shrink), re-extract the symbol table and update.

set -uo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
VBLS=${VBLS:-6000}
TOS=${TOS_IMG:-/Users/sarnau/Desktop/Retro/Atari\ ST/Atari\ TOS\ Images/TOS104US.ROM}
GAME=$HOME/hatari-c/GAME
PRG=$CSRC/build/alcyon/LCP.PRG

if [ -z "${NO_REBUILD:-}" ]; then
    ALCYON_CPPFLAGS="-DSKIP_TITLE=1" "$CSRC/tools/alcyon_build.sh" >/dev/null 2>&1 \
        && "$CSRC/tools/alcyon_link.sh" >/dev/null 2>&1 \
        || { echo "SETUP: rebuild for tests failed" >&2; exit 2; }
fi

if [ ! -f "$PRG" ]; then
    echo "SETUP: LCP.PRG not found at $PRG (build first: make alcyon)" >&2
    exit 2
fi

# ---- Runtime symbol addresses ------------------------------------------
# Text base of the loaded PRG (established from `info basepage` in Hatari).
# Alcyon links this build with a static image base — re-check with the
# `find_base.sh` helper if the layout changes.
BASE=0x13c14

# Symbol offsets -- re-derive after any BSS/DATA/TEXT drift with:
#   python3 tools/find_syms.py _lcp_x _lcp_y _g_wtx _g_wty _lcp_st \
#                                                    _g_wyx _g_wyy _lcp_stR
LCP_X=$((BASE + 0x1919c))
LCP_Y=$((BASE + 0x1919e))
G_WTX=$((BASE + 0x191bc))
G_WTY=$((BASE + 0x191be))
LCP_ST=$((BASE + 0x1aa00))
G_WYX=$((BASE + 0x1af5c))
G_WYY=$((BASE + 0x1af5e))
LCP_STR=$((BASE + 0x1af60))

printf -v LCP_X_H '%x'  $LCP_X
printf -v LCP_Y_H '%x'  $LCP_Y
printf -v G_WTX_H '%x'  $G_WTX
printf -v G_WTY_H '%x'  $G_WTY
printf -v G_WYX_H '%x'  $G_WYX
printf -v G_WYY_H '%x'  $G_WYY
printf -v LCP_ST_H '%x' $LCP_ST
printf -v STR_H '%x'    $LCP_STR

# Attic centre / upper-flight top: (x=182, y=72) matches stair_wp[4..5].
# Bottom-floor centre: (x=300, y=195).
INIT_VBL=1500
# First sample lands just after the warp so we capture the top-floor
# start state before lcp_wkD begins moving.
SAMPLES="1520 1600 1800 2100 2500 3000 3500 4000 5000"

WORKDIR=$(mktemp -d)
trap "rm -rf $WORKDIR" EXIT

# Hatari `w` writes a single byte.  Set each 16-bit field one byte
# at a time.  Coordinates go big-endian (68k native).
# lcp_x = 182 (0x00b6), lcp_y = 72 (0x0048)
# g_wtx = 300 (0x012c), g_wty = 195 (0x00c3)
# g_wyx = 0, g_wyy = 0, lcp_stR = 0
cat > "$WORKDIR/warp.dbg" <<EOF
w \$$LCP_X_H \$00
w \$$(printf '%x' $((LCP_X + 1))) \$b6
w \$$LCP_Y_H \$00
w \$$(printf '%x' $((LCP_Y + 1))) \$48
w \$$G_WTX_H \$01
w \$$(printf '%x' $((G_WTX + 1))) \$2c
w \$$G_WTY_H \$00
w \$$(printf '%x' $((G_WTY + 1))) \$c3
w \$$G_WYX_H \$00
w \$$(printf '%x' $((G_WYX + 1))) \$00
w \$$G_WYY_H \$00
w \$$(printf '%x' $((G_WYY + 1))) \$00
w \$$STR_H   \$00
w \$$(printf '%x' $((LCP_STR + 1))) \$00
c
EOF

# Sample script: dump 6 shorts covering lcp_x/y, walk target, walk waypoint,
# and lcp_st/stR.  Called each sample point.
cat > "$WORKDIR/sample.dbg" <<EOF
m \$$LCP_X_H-\$$(printf '%x' $((LCP_X + 4)))
m \$$G_WTX_H-\$$(printf '%x' $((G_WTX + 4)))
m \$$LCP_ST_H-\$$(printf '%x' $((LCP_ST + 2)))
m \$$G_WYX_H-\$$(printf '%x' $((G_WYX + 6)))
c
EOF

# Compose the .ini: install the warp at $INIT_VBL, then a series of
# sample bps at the given VBL points.
{
    echo "b VBL > $INIT_VBL :once :file $WORKDIR/warp.dbg"
    for v in $SAMPLES; do
        echo "b VBL > $v :once :file $WORKDIR/sample.dbg"
    done
    echo "c"
} > "$WORKDIR/run.ini"

cp -f "$PRG" "$GAME/LCP.PRG"
rm -f "$GAME/LCP.SAV"

pkill -x hatari 2>/dev/null; sleep 1

hatari \
    --harddrive "$GAME" \
    --tos "$TOS" \
    --fast-forward on \
    --run-vbls "$VBLS" \
    --parse "$WORKDIR/run.ini" > "$WORKDIR/hatari.log" 2>&1 &
HPID=$!
sleep 40
kill $HPID 2>/dev/null; wait $HPID 2>/dev/null

# Extract sample values.
python3 - "$WORKDIR/hatari.log" <<'PY'
import re, sys

log = open(sys.argv[1]).read()
blocks = re.split(r'CPU breakpoint condition\(s\) matched \d+ times', log)[1:]

samples = []
for b in blocks:
    lx  = re.search(r'0002CDB0: ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2})', b)
    wt  = re.search(r'0002CDD0: ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2})', b)
    st  = re.search(r'0002E614: ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2})', b)
    wy  = re.search(r'0002EB70: ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2})', b)
    if lx and wt and st and wy:
        samples.append({
            'lx':  int(lx.group(1)+lx.group(2), 16),
            'ly':  int(lx.group(3)+lx.group(4), 16),
            'wtx': int(wt.group(1)+wt.group(2), 16),
            'wty': int(wt.group(3)+wt.group(4), 16),
            'st':  int(st.group(1)+st.group(2), 16),
            'wyx': int(wy.group(1)+wy.group(2), 16),
            'wyy': int(wy.group(3)+wy.group(4), 16),
            'stR': int(wy.group(5)+wy.group(6), 16),
        })

# The first block is the warp itself (its `c` inside warp.dbg dumps
# nothing) — samples[0] is our first real sample after warp.

if not samples:
    print('FAIL: no samples captured (bp never fired)')
    sys.exit(2)

def floor_of(y):
    if y < 78:  return 3
    if y < 141: return 2
    return 1

print("VBL sequence:")
print(f"  {'#':>2}  {'lcp':>10}  {'st':>3}  {'stR':>3}  {'wt':>10}  {'wy':>10}  {'floor':>5}")
for i, s in enumerate(samples):
    fl = floor_of(s['ly'])
    print(f"  {i:>2}  ({s['lx']:3d},{s['ly']:3d})  {s['st']:3d}  {s['stR']:3d}  ({s['wtx']:3d},{s['wty']:3d})  ({s['wyx']:3d},{s['wyy']:3d})  {fl:>5}")

# ---- Assertions -------------------------------------------------------
errs = []

# 1. First sample should catch the resident on the top floor (post-warp,
#    before lcp_wkD steps them anywhere).
if floor_of(samples[0]['ly']) != 3:
    errs.append(f"start floor {floor_of(samples[0]['ly'])}, expected 3 (top)")

# 2. Last sample should have arrived on bottom floor.
if floor_of(samples[-1]['ly']) != 1:
    errs.append(f"end floor {floor_of(samples[-1]['ly'])}, expected 1 (bottom)")

# 3. Somewhere in the sequence, lcp_stR should be YES (=1) —
#    proving stair mode was engaged.
if not any(s['stR'] == 1 for s in samples):
    errs.append("lcp_stR never became 1 — resident did not enter stair mode")

# 4. Somewhere lcp_st should be in the stair-state ranges
#    (STATE_STAIR_CLIMB_FRAME_0..STATE_STAIR_BTM_FRAME_3 = 9..24).
if not any(9 <= s['st'] <= 24 for s in samples):
    errs.append("lcp_st never entered the 9..24 stair-state range")

# 5. Resident should traverse floor 2 on the way down — checking that
#    the boundary crossings actually happened (rather than a teleport).
floors_seen = {floor_of(s['ly']) for s in samples}
if 2 not in floors_seen:
    errs.append("resident never appeared on floor 2 — likely teleported past it")

# 6. Y should have moved monotonically (with the animation snaps of
#    +2/-2/+6/-6 the game does at flight transitions).  A sudden drop
#    of more than ~40 game-pixels between adjacent samples that isn't
#    followed by any stair state indicates a fall.
for i in range(1, len(samples)):
    dy = samples[i]['ly'] - samples[i-1]['ly']
    if dy > 40 and samples[i-1]['stR'] == 0 and samples[i]['stR'] == 0:
        errs.append(
            f"between sample {i-1} and {i}, y jumped by {dy} while "
            f"lcp_stR stayed 0 (fall through floor)"
        )

if errs:
    print("\nFAIL:")
    for e in errs:
        print(f"  - {e}")
    sys.exit(1)

print("\nPASS: top→bottom stair descent completed correctly.")
sys.exit(0)
PY
rc=$?

echo ""
if [ "$rc" = "0" ]; then
    echo "==== STAIR DESCENT TEST ===="
    echo "VERDICT:     PASS"
else
    echo "==== STAIR DESCENT TEST ===="
    echo "VERDICT:     FAIL"
fi
exit $rc
