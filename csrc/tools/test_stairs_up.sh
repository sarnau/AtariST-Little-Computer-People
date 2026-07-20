#!/usr/bin/env bash
# test_stairs_up.sh -- verify the resident can ascend from the bottom
# floor (kitchen/living room) to the top floor (attic) via both
# staircases without skipping a floor.
#
# Symmetric counterpart to test_stairs.sh; see that script for method
# and address-offset provenance.
#
# Env:
#   VBLS       total run length     (default 6000)
#   TOS_IMG    TOS ROM to boot      (see run_hatari.sh defaults)
#
# Exit codes:
#   0  ascent verified
#   1  ascent failed
#   2  setup error / bp never fired

set -uo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
VBLS=${VBLS:-6000}
TOS=${TOS_IMG:-/Users/sarnau/Desktop/Retro/Atari\ ST/Atari\ TOS\ Images/TOS104US.ROM}
GAME=$HOME/hatari-c/GAME
PRG=$CSRC/build/alcyon/LCP.PRG

if [ -z "${NO_REBUILD:-}" ]; then
    ALCYON_CPPFLAGS="-DTEST_STAIRS=2 -DSKIP_TITLE=1 -DSKIP_MIDI=1" \
        FILES=init.c "$CSRC/tools/alcyon_build.sh" >/dev/null 2>&1 \
        && "$CSRC/tools/alcyon_link.sh" >/dev/null 2>&1 \
        || { echo "SETUP: rebuild for tests failed" >&2; exit 2; }
fi

if [ ! -f "$PRG" ]; then
    echo "SETUP: LCP.PRG not found at $PRG (build first: make alcyon)" >&2
    exit 2
fi

BASE=0x13c14
# Derive symbol offsets from the just-built .o files -- see
# test_stairs.sh for rationale.
_syms=$(python3 "$CSRC/tools/find_syms.py" \
        _lcp_x _lcp_y _g_wtx _g_wty _lcp_st _g_wyx _g_wyy _lcp_stR)
_off() { echo "$_syms" | awk -v s="_$1" '$1==s { print $3 }' | sed 's/base+//'; }
LCP_X=$((BASE + $(_off lcp_x)))
LCP_Y=$((BASE + $(_off lcp_y)))
G_WTX=$((BASE + $(_off g_wtx)))
G_WTY=$((BASE + $(_off g_wty)))
LCP_ST=$((BASE + $(_off lcp_st)))
G_WYX=$((BASE + $(_off g_wyx)))
G_WYY=$((BASE + $(_off g_wyy)))
LCP_STR=$((BASE + $(_off lcp_stR)))

printf -v LCP_X_H '%x'  $LCP_X
printf -v LCP_Y_H '%x'  $LCP_Y
printf -v G_WTX_H '%x'  $G_WTX
printf -v G_WTY_H '%x'  $G_WTY
printf -v G_WYX_H '%x'  $G_WYX
printf -v G_WYY_H '%x'  $G_WYY
printf -v LCP_ST_H '%x' $LCP_ST
printf -v STR_H '%x'    $LCP_STR

# Warp: bottom-floor stair entry (170, 185) with attic centre target
# (300, 45).  x=170 (0x00aa), y=185 (0x00b9), wtx=300 (0x012c),
# wty=45 (0x002d).
INIT_VBL=1500
# Dense sampling — climbing crosses two flights of stairs plus the
# middle-floor landing in ~1500 VBLs; too-sparse a schedule can leave
# the "fall through ceiling" heuristic seeing a large Y-jump between
# samples with lcp_stR=0 on both endpoints (legitimate climb).
SAMPLES="1520 1600 1750 1900 2100 2300 2500 2700 2900 3100 3300 3600 4000 5000"

WORKDIR=$(mktemp -d)
trap "rm -rf $WORKDIR" EXIT

# Warp + lcp_wkD is done in-C via the TEST_STAIRS=2 hook in cs_mvIn
# (init.c).  Sampling only from here.

cat > "$WORKDIR/sample.dbg" <<EOF
m \$$LCP_X_H-\$$(printf '%x' $((LCP_X + 4)))
m \$$G_WTX_H-\$$(printf '%x' $((G_WTX + 4)))
m \$$LCP_ST_H-\$$(printf '%x' $((LCP_ST + 2)))
m \$$G_WYX_H-\$$(printf '%x' $((G_WYX + 6)))
c
EOF

{
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

LCP_X_H_UP=$(printf '%08X' $LCP_X)
G_WTX_H_UP=$(printf '%08X' $G_WTX)
LCP_ST_H_UP=$(printf '%08X' $LCP_ST)
G_WYX_H_UP=$(printf '%08X' $G_WYX)

LCP_X_ADDR=$LCP_X_H_UP G_WTX_ADDR=$G_WTX_H_UP \
LCP_ST_ADDR=$LCP_ST_H_UP G_WYX_ADDR=$G_WYX_H_UP \
python3 - "$WORKDIR/hatari.log" <<'PY'
import os, re, sys

log = open(sys.argv[1]).read()
blocks = re.split(r'CPU breakpoint condition\(s\) matched \d+ times', log)[1:]

LX = os.environ['LCP_X_ADDR']
WT = os.environ['G_WTX_ADDR']
ST = os.environ['LCP_ST_ADDR']
WY = os.environ['G_WYX_ADDR']

samples = []
for b in blocks:
    lx  = re.search(LX + r': ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2})', b)
    wt  = re.search(WT + r': ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2})', b)
    st  = re.search(ST + r': ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2})', b)
    wy  = re.search(WY + r': ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2}) ([0-9a-fA-F]{2})', b)
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

errs = []

# 1. First sample should catch the resident on the bottom floor.
if floor_of(samples[0]['ly']) != 1:
    errs.append(f"start floor {floor_of(samples[0]['ly'])}, expected 1 (bottom)")

# 2. Last sample should have arrived on top floor.
if floor_of(samples[-1]['ly']) != 3:
    errs.append(f"end floor {floor_of(samples[-1]['ly'])}, expected 3 (top)")

# 3. Somewhere in the sequence, lcp_stR should be YES (=1).
if not any(s['stR'] == 1 for s in samples):
    errs.append("lcp_stR never became 1 — resident did not enter stair mode")

# 4. Somewhere lcp_st should be in the stair-state range (9..24).
if not any(9 <= s['st'] <= 24 for s in samples):
    errs.append("lcp_st never entered the 9..24 stair-state range")

# 5. Resident should traverse floor 2 on the way up.
floors_seen = {floor_of(s['ly']) for s in samples}
if 2 not in floors_seen:
    errs.append("resident never appeared on floor 2 — likely teleported past it")

# 6. Big negative Y jumps (>40) with lcp_stR=0 on both endpoints
#    indicate a fall through the ceiling.
for i in range(1, len(samples)):
    dy = samples[i-1]['ly'] - samples[i]['ly']
    if dy > 40 and samples[i-1]['stR'] == 0 and samples[i]['stR'] == 0:
        errs.append(
            f"between sample {i-1} and {i}, y jumped up by {dy} while "
            f"lcp_stR stayed 0 (teleport through ceiling)"
        )

if errs:
    print("\nFAIL:")
    for e in errs:
        print(f"  - {e}")
    sys.exit(1)

print("\nPASS: bottom→top stair ascent completed correctly.")
sys.exit(0)
PY
rc=$?

echo ""
if [ "$rc" = "0" ]; then
    echo "==== STAIR ASCENT TEST ===="
    echo "VERDICT:     PASS"
else
    echo "==== STAIR ASCENT TEST ===="
    echo "VERDICT:     FAIL"
fi

# Rebuild init.c without TEST_STAIRS so subsequent runs use a
# clean binary.
FILES=init.c "$CSRC/tools/alcyon_build.sh" > /dev/null 2>&1

exit $rc
