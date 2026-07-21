#!/usr/bin/env bash
# trace_ssco_caller.sh -- identify which action calls sp_ssco and
# leaks g_lcyof = YES.
#
# Method:
#   - Breakpoint at sp_ssco entry: dump the return address on the
#     stack (a7).  That points into the caller function's next
#     instruction, so we can resolve it to an action.
#   - Value-change memwatch on g_lcyof.  Correlate: every
#     g_lcyof=YES write happens INSIDE sp_ssco, so the most recent
#     sp_ssco-entry return address before the write is the caller
#     that set the flag.  Every g_lcyof=NO write happens in the
#     caller (post-sp_ssco), so if a caller pattern shows sp_ssco
#     entries without a matching NO write, that's the leak.
#
# Under --fast-forward at ~1700 VBL/s a ~1-hour game session runs in
# ~2 min host time.  Freeze-detector cap of 2 stagnant 60s samples.
#
# Usage:  source/tools/trace_ssco_caller.sh
# Log:    /tmp/lcp_ssco_trace.log

set -uo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
TOS_IMG=${TOS_IMG:-/Users/sarnau/Desktop/Retro/Atari ST/Atari TOS Images/TOS104US.ROM}
GAME_DIR=${GAME_DIR:-$HOME/hatari-c/GAME}
PRG=${PRG:-$CSRC/build/alcyon/LCP.PRG}
LOG=/tmp/lcp_ssco_trace.log
VBLS=${VBLS:-250000}

[ -f "$PRG" ]     || { echo "SETUP: LCP.PRG missing at $PRG" >&2; exit 2; }
[ -f "$TOS_IMG" ] || { echo "SETUP: TOS ROM missing at $TOS_IMG" >&2; exit 2; }

BASE=0x12596
# sp_ssco entry offset in sprites.o TEXT (auto-derived).
SSCO_ENTRY=$(python3 - <<PY
import struct, os
BUILD = "$CSRC/build/alcyon"
files = sorted(f for f in os.listdir(BUILD) if f.endswith('.o'))
special = ['gemstart.o', 'main.o']
skip = {'osbind.o', 'crt0.o', 'nofloat.o'}
ordered = special + [f for f in files if f not in special and f not in skip]
acc_t = 0
for f in ordered:
    with open(f'{BUILD}/{f}', 'rb') as h: d = h.read()
    _m, tlen, dlen, _b, sl = struct.unpack('>HIIII', d[:0x12])
    if f == 'sprites.o':
        i, end = 0x1c + tlen + dlen, 0x1c + tlen + dlen + sl
        while i < end:
            e = d[i:i+14]
            name = e[:8].rstrip(b'\0').decode('latin1', 'replace')
            typ, val = struct.unpack('>HI', e[8:14])
            if (typ & 0x0048) == 0x0048 and i + 28 <= end:
                name += d[i+14:i+28].rstrip(b'\0').decode('latin1', 'replace')
                i += 28
            else:
                i += 14
            if name == '_sp_ssco' and (typ & 0xa000) == 0xa000 and (typ & 0x0e00) == 0x0200:
                print(f'{$BASE + acc_t + val:x}')
                break
        break
    acc_t += tlen
PY
)
LCYOF_ADDR=$(printf '%x' $((BASE + 0x1c2d4)))

echo "sp_ssco entry: \$$SSCO_ENTRY"
echo "g_lcyof:       \$$LCYOF_ADDR"
echo "log:           $LOG"
echo ""

WORKDIR=$(mktemp -d)
trap "rm -rf $WORKDIR" EXIT

# sp_ssco entry dump: return address = *(long*)a7.  cpureg on
# entry shows registers including a7; we specifically need the
# 4 bytes at (a7).
cat > "$WORKDIR/ssco.dbg" <<EOF
memdump "(a7)"-"(a7)+3"
EOF

# g_lcyof change dump: show a marker.  We don't need PC history --
# the sp_ssco entry breakpoint already logged the caller.
cat > "$WORKDIR/lcyof.dbg" <<EOF
EOF

cat > "$WORKDIR/run.ini" <<EOF
b pc = \$$SSCO_ENTRY :trace :file $WORKDIR/ssco.dbg
b (\$$LCYOF_ADDR).w ! (\$$LCYOF_ADDR).w :trace :file $WORKDIR/lcyof.dbg
c
EOF

cp -f "$PRG" "$GAME_DIR/LCP.PRG"
rm -f "$GAME_DIR/LCP.SAV" "$GAME_DIR/HYBER"
pkill -x hatari 2>/dev/null; sleep 1

hatari \
    --harddrive "$GAME_DIR" \
    --tos "$TOS_IMG" \
    --fast-forward on \
    --auto 'C:\LCP.PRG' \
    --run-vbls "$VBLS" \
    --parse "$WORKDIR/run.ini" > "$LOG" 2>&1 &
HPID=$!

# Freeze detector.
prev=0; stag=0
while kill -0 $HPID 2>/dev/null; do
    sleep 60
    cur=$(wc -c < "$LOG" 2>/dev/null | tr -d ' '); cur=${cur:-0}
    if [ "$cur" = "$prev" ]; then
        stag=$((stag + 1))
        echo "[monitor] log stagnant ($stag/2) at $cur bytes" >&2
        [ $stag -ge 2 ] && { echo "[monitor] terminating Hatari"; kill $HPID 2>/dev/null; break; }
    else
        stag=0
        echo "[monitor] $prev -> $cur bytes" >&2
    fi
    prev=$cur
done
wait $HPID 2>/dev/null
