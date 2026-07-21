#!/usr/bin/env bash
# catch_lcp_std_writer.sh -- capture whatever wild pointer is writing
# into lcp_std's compiled TEXT region.
#
# The intermittent real-time-only TOS VDI bus/address error appears to
# stem from a wild write into lcp_std's function body -- specifically
# clobbering an abs.L operand in one of its instructions.  This script
# arms a Hatari memory-write breakpoint over the entire lcp_std body,
# then hands control back to you so you can play the game normally
# under real-time (NOT --fast-forward).  When the wild write fires,
# Hatari drops into its debugger and dumps the writer's PC + registers
# + a context window; use `c` to continue, or `q` to exit and share
# the log.
#
# The lcp_std runtime range is re-derived from the current build each
# time, so a fresh rebuild doesn't invalidate the memwatch.
#
# Prereqs:
#   - source/build/alcyon/LCP.PRG (any config -- production build
#     recommended: no SKIP_TITLE, no SKIP_MIDI, no TEST_* hooks)
#   - Hatari installed, TOS 1.04 ROM at the usual path (env overridable)
#
# Env:
#   TOS_IMG    TOS ROM                   (default TOS104US.ROM in Retro/)
#   GAME_DIR   Hatari GEMDOS-HDD root    (default ~/hatari-c/GAME)
#   PRG        LCP.PRG to install        (default source/build/alcyon/LCP.PRG)
#
# Usage:
#   source/tools/catch_lcp_std_writer.sh
#
# Then play the game as you normally would.  When it crashes, Hatari's
# debugger will stop with the writer's PC on screen; press 'q' to
# exit and paste the last ~50 lines of /tmp/lcp_memwatch.log to me
# for analysis.

set -uo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
TOS_IMG=${TOS_IMG:-/Users/sarnau/Desktop/Retro/Atari ST/Atari TOS Images/TOS104US.ROM}
GAME_DIR=${GAME_DIR:-$HOME/hatari-c/GAME}
PRG=${PRG:-$CSRC/build/alcyon/LCP.PRG}
LOG=/tmp/lcp_memwatch.log

[ -f "$PRG" ]     || { echo "SETUP: LCP.PRG missing at $PRG" >&2; exit 2; }
[ -f "$TOS_IMG" ] || { echo "SETUP: TOS ROM missing at $TOS_IMG" >&2; exit 2; }

# Derive lcp_std's runtime range from the current build.
read LCP_STD_START LCP_STD_END <<< "$(python3 - <<'PY'
import struct, os, sys
BUILD = os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])),
                     '..', 'build', 'alcyon')
BUILD = os.path.abspath(BUILD)
files = sorted(f for f in os.listdir(BUILD) if f.endswith('.o'))
special = ['gemstart.o', 'main.o']
skip = {'osbind.o', 'crt0.o', 'nofloat.o'}
ordered = special + [f for f in files if f not in special and f not in skip]

# Compute save.o's TEXT offset in the linked PRG
acc_t = 0
save_o_text = None
for f in ordered:
    with open(f'{BUILD}/{f}', 'rb') as h: d = h.read()
    _m, tlen, _dl, _bl, _sl = struct.unpack('>HIIII', d[:0x12])
    if f == 'save.o':
        save_o_text = acc_t
    acc_t += tlen

# Find lcp_std intra-.o offset (typ 0xa200 = defined+global+TEXT).
with open(f'{BUILD}/save.o', 'rb') as f: d = f.read()
_m, tlen, dlen, blen, symlen = struct.unpack('>HIIII', d[:0x12])
sym_off = 0x1c + tlen + dlen
i, end = sym_off, sym_off + symlen
text_syms = []
while i < end:
    e = d[i:i+14]
    name = e[:8].rstrip(b'\0').decode('latin1', 'replace')
    typ, val = struct.unpack('>HI', e[8:14])
    if (typ & 0x0048) == 0x0048 and i + 28 <= end:
        name += d[i+14:i+28].rstrip(b'\0').decode('latin1', 'replace')
        i += 28
    else:
        i += 14
    if (typ & 0xa000) == 0xa000 and (typ & 0x0e00) == 0x0200:
        text_syms.append((val, name))
text_syms.sort()

BASE = 0x12596  # --auto load address under TOS 1.04
for j, (v, n) in enumerate(text_syms):
    if n == '_lcp_std':
        next_v = text_syms[j+1][0] if j+1 < len(text_syms) else tlen
        rt_start = BASE + save_o_text + v
        rt_end   = BASE + save_o_text + next_v
        print(f'{rt_start:x} {rt_end:x}')
        break
PY
)"

if [ -z "${LCP_STD_START:-}" ]; then
    echo "SETUP: failed to derive lcp_std runtime range" >&2
    exit 2
fi

echo "==== lcp_std memwatch armed ===="
echo "  runtime range: \$${LCP_STD_START}..\$${LCP_STD_END}"
echo "  log file:      $LOG"
echo ""
echo "Play the game normally.  When the crash fires the debugger will"
echo "stop with the writer's PC visible; press 'q' to quit and share"
echo "the last ~50 lines of $LOG with me."
echo ""

WORKDIR=$(mktemp -d)
trap "rm -rf $WORKDIR" EXIT

# Memwatch on any write into lcp_std's TEXT.  Hatari's Access(range,type)
# expression fires on any write inside the range; :trace dumps state on
# hit but doesn't stop emulation (so a false-positive doesn't kill the
# session).  Actual stop happens later when the corrupted operand
# executes.
cat > "$WORKDIR/watch.dbg" <<EOF
info registers
disasm PC-20 30
history
EOF

cat > "$WORKDIR/run.ini" <<EOF
b (\$${LCP_STD_START} <= Access(w)) && (Access(w) < \$${LCP_STD_END}) :trace :file $WORKDIR/watch.dbg
c
EOF

cp -f "$PRG" "$GAME_DIR/LCP.PRG"
rm -f "$GAME_DIR/LCP.SAV" "$GAME_DIR/HYBER"
pkill -x hatari 2>/dev/null; sleep 1

hatari \
    --harddrive "$GAME_DIR" \
    --tos "$TOS_IMG" \
    --auto 'C:\LCP.PRG' \
    --parse "$WORKDIR/run.ini" 2>&1 | tee "$LOG"
