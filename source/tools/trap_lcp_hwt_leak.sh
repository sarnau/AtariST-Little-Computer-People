#!/usr/bin/env bash
# trap_lcp_hwt_leak.sh -- catch every call into lcp_hwt while
# g_lcyof != 0 and dump the return address on the stack (that's
# the caller's PC-after-jsr).  Correlated with the linker map,
# this names the exact function that leaks g_lcyof into lcp_hwt.
#
# lcp_hwt busy-waits for g_hacur == g_hatas.  When g_lcyof != 0
# gameTick's Path B runs, sp_lcha never fires, g_hacur is frozen,
# lcp_hwt loops forever.  The ROM never enters this state, so
# whichever port function calls lcp_hwt without first clearing
# g_lcyof is where the port diverges from ROM behavior.
#
# lcp_hwt runtime: $2667a; g_lcyof: $2e86e (from lcp.map).
#
# Usage:  source/tools/trap_lcp_hwt_leak.sh
# Log:    /tmp/lcp_hwt_leak.log

set -uo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
TOS_IMG=${TOS_IMG:-/Users/sarnau/Desktop/Retro/Atari ST/Atari TOS Images/TOS104US.ROM}
GAME_DIR=${GAME_DIR:-$HOME/Hatari_C/hatari-c/GAME}
PRG=${PRG:-$CSRC/build/alcyon/LCP.PRG}
LOG=/tmp/lcp_hwt_leak.log
VBLS=${VBLS:-30000}     # stall fires early; no need for 250k

[ -f "$PRG" ]     || { echo "SETUP: LCP.PRG missing" >&2; exit 2; }
[ -f "$TOS_IMG" ] || { echo "SETUP: TOS ROM missing" >&2; exit 2; }

WORKDIR=$(mktemp -d)
trap "rm -rf $WORKDIR" EXIT

# On hit: dump 4 bytes at (a7) -- the caller's return address that
# jsr pushed.  Only the first hit matters (once the stall starts
# every subsequent lcp_hwt entry is with g_lcyof=YES too).
cat > "$WORKDIR/hit.dbg" <<'EOF'
memdump "(a7)"-"(a7)+3"
EOF

cat > "$WORKDIR/run.ini" <<EOF
b pc = \$2667a && (\$2e86e).w > 0 :once :file $WORKDIR/hit.dbg
c
EOF

cp -f "$PRG" "$GAME_DIR/LCP.PRG"
rm -f "$GAME_DIR/LCP.SAV" "$GAME_DIR/HYBER"
pkill -x hatari 2>/dev/null; sleep 1

hatari --harddrive "$GAME_DIR" --tos "$TOS_IMG" --fast-forward on \
    --auto 'C:\LCP.PRG' --run-vbls "$VBLS" \
    --parse "$WORKDIR/run.ini" > "$LOG" 2>&1

# Extract the return address from the memdump
RA=$(grep -oE "'\(a7\)' -> \\\$[0-9a-fA-F]+" "$LOG" | head -1 | grep -oE '[0-9a-fA-F]+$')
if [ -z "$RA" ]; then
    RA=$(grep -oE '^0002667A|^[0-9A-F]{8}: [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2} [0-9A-F]{2}' "$LOG" | head -1)
    echo "raw hit region:"
    grep -B 2 -A 4 "memdump" "$LOG" | head -20
fi

# Try both syntaxes for return-addr extraction
RA=$(python3 -c "
import re
log = open('$LOG').read()
# Look for memdump output of (a7) after the hit
m = re.search(r'memdump.*\n[0-9A-Fa-f]{8}:\s+([0-9A-Fa-f]{2})\s+([0-9A-Fa-f]{2})\s+([0-9A-Fa-f]{2})\s+([0-9A-Fa-f]{2})', log)
if m:
    print(''.join(m.groups()))
")
if [ -n "$RA" ]; then
    echo ""
    echo "return address on stack: 0x$RA"
    python3 <<PY
target = int("$RA", 16)
map_path = '$CSRC/build/alcyon/lcp.map'
BASE = 0x12596
syms = []
for line in open(map_path).read().splitlines():
    if line.startswith('#') or not line.strip(): continue
    parts = line.split()
    if len(parts) < 4: continue
    name, seg = parts[0], parts[1]
    if seg == 'T':
        syms.append((int(parts[3], 16), name))
syms.sort()
below = [s for s in syms if s[0] <= target]
if below:
    r, n = below[-1]
    print(f'  caller PC 0x{target:x} = {n} @ 0x{r:x}, offset +0x{target-r:x}')
above = [s for s in syms if s[0] > target]
if above:
    print(f'  next symbol: {above[0][1]} @ 0x{above[0][0]:x}')
PY
else
    echo ""
    echo "no hit captured -- log tail:"
    tail -20 "$LOG"
fi
