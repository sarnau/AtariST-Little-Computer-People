#!/usr/bin/env bash
#
# test_actions.sh -- type every command the parser can accept and check
# it produces the action its own tables say it should.
#
# This replaces a version that DID NOT RUN.  It built with
# -DTEST_ACTIONS=n to switch on a harness in cs_mvIn that pushed one
# event into g_trel; that harness was removed during the LCP_STX
# restructuring, so the flag compiled to nothing and the script
# reported success while exercising no hook.  Its action-ID table had
# also gone stale against enums.h -- it listed ACTION_HELLO as 18 where
# the enum says 24 -- which is the other reason not to resurrect it.
#
# WHAT IS UNDER TEST is chk_encm, the runtime matcher, against the
# static tables it reads.  The expected command/action pairs are
# DERIVED from vocab data at run time rather than hard-coded, so the
# test cannot go stale the way the old ID list did: change the tables
# and the expectations follow.  What it catches is the engine drifting
# from the data -- a wrong mask test, a wrong first-match rule, a
# broken bit accumulation.
#
# How the parser works: each recognised word ORs bm_lo[g_ew2b[w]] into
# g_ewb[ew2pos[w]]; each g_ew2a row is a 10-byte mask plus an action at
# +10; a row fires when its mask is a SUBSET of the accumulated bits,
# first match winning.  prsCmd appends the result to g_aqueu[g_aliss].
#
# Two rows can never fire and the script asserts that they do not:
#   row 0  ACTION_HELLO -- needs byte 9 bit 0x01 and no word supplies it
#   row 6  MESSY IS HOME -- needs the `IS` at vwd_tab index 84, a
#          duplicate that chk_vwd can never return
#
# Env: NO_REBUILD=1, KEEP_LOG=1, HATARI=, TOS_IMG=, GAME_DIR=.
# Exit: 0 all as predicted, 1 at least one mismatch, 2 setup error.

set -uo pipefail
CSRC=$(cd "$(dirname "$0")/.." && pwd)
. "$CSRC/tools/hatari_probe.sh"

# ---- derive the expectations from the shipped tables -----------------
TABLE=$(python3 - "$CSRC/dat_u3b.c" "$CSRC/include/enums.h" <<'PY'
import re, sys
src = open(sys.argv[1]).read()
def nums(name, count):
    m = re.search(re.escape(name) + r'\s*\[\s*\d*\s*\]\s*=\s*\{(.*?)\};', src, re.S)
    body = re.sub(r'/\*.*?\*/', '', m.group(1), flags=re.S)
    toks = re.findall(r'0[xX][0-9a-fA-F]+|-?\d+', body)
    return [int(t, 16) if t.lower().startswith('0x') else int(t, 10) for t in toks][:count]
def strs(name):
    m = re.search(re.escape(name) + r'\s*\[\s*\d*\s*\]\s*=\s*\{(.*?)\};', src, re.S)
    return re.findall(r'"([^"]*)"', re.sub(r'/\*.*?\*/', '', m.group(1), flags=re.S))

ew2pos = nums('ew2pos', 161); g_ew2b = nums('g_ew2b', 160)
bm_lo  = nums('bm_lo', 8);    words  = strs('vwd_tab')
flat   = nums('g_ew2a', 34 * 12)
rows   = [flat[i*12:(i+1)*12] for i in range(34)]

# chk_vwd returns the FIRST spelling match, and chk_encm reads a return
# of 0 as "unrecognised" -- so index 0 and every repeated spelling are
# unreachable and must not be used to build a command.
live = {}
for i, w in enumerate(words):
    if i == 0 or w in live:
        continue
    live[w] = i
prov = {}
for w, i in live.items():
    prov.setdefault((ew2pos[i] & 0xff, bm_lo[g_ew2b[i] & 0xff]), []).append(w)

for idx, r in enumerate(rows):
    if r[0] & 0xff == 0xff:
        break
    need = set()
    for i, b in enumerate(r[:10]):
        b &= 0xff
        for k in range(8):
            if b & (1 << k):
                need.add((i, 1 << k))
    sel, acc, ok = [], [0]*10, True
    for key in sorted(need):
        if acc[key[0]] & key[1]:
            continue
        c = prov.get(key)
        if not c:
            ok = False
            break
        w = sorted(c, key=len)[0]
        sel.append(w)
        i = live[w]
        acc[ew2pos[i] & 0xff] |= bm_lo[g_ew2b[i] & 0xff]
    if not ok:
        # Unreachable: build the nearest command anyway so the script
        # can assert that typing it yields NOTHING.
        alt = []
        for key in sorted(need):
            c = prov.get(key)
            if c:
                alt.append(sorted(c, key=len)[0])
        print('%d|-1|%s' % (idx, ' '.join(dict.fromkeys(alt)) or 'HELLO'))
        continue
    first = next((j for j, r2 in enumerate(rows)
                  if (r2[0] & 0xff) != 0xff
                  and all((r2[i] & 0xff) & acc[i] == (r2[i] & 0xff) for i in range(10))), None)
    print('%d|%d|%s' % (idx, rows[first][10] & 0xff, ' '.join(sel)))
PY
) || { echo "SETUP: cannot derive the command table" >&2; exit 2; }

[ -z "$TABLE" ] && { echo "SETUP: derived table is empty" >&2; exit 2; }

probe_start
A_ALISS=$(probe_addr _g_aliss)
A_AQUEU=$(probe_addr _g_aqueu)

echo "load base \$$(probe_base);  $(echo "$TABLE" | wc -l | tr -d ' ') rows derived"
echo ""

# Measure at 1x: fast-forward would let the AI drain the queue and the
# text buffer time out between the Return and the read.
probe_fast off

pass=0; fail=0; results=""

# The queue is only 10 deep and prsCmd silently DROPS anything past
# that, so 33 commands cannot simply be poured in and read back -- and
# waiting for the resident to work each one off would take the whole
# game day.  Reset g_aliss to empty before every command instead: what
# the command produced is then unambiguously slot 0, and the queue can
# never fill.  Discarding queued actions costs this test nothing; it is
# the PARSER under test, not the AI.
# Keystrokes go into the IKBD buffer far faster than the game takes
# them out -- it reads at most one per tick -- so a 24-character
# command is still being consumed long after the last key was injected.
# A fixed sleep therefore reads the PREVIOUS command's result and looks
# exactly like an off-by-one in the parser.  Poll instead.
run_one() {                              # $1 row, $2 want, $3 cmd
    local n
    probe_poke "$A_ALISS" 0 0
    probe_cmd "$3"
    for _ in $(seq 1 30); do
        n=$(probe_word "$A_ALISS")
        [ -n "$n" ] && [ "$n" -ne 0 ] && { echo "$n"; return 0; }
        sleep 0.2
    done
    echo 0
}

# An unreachable row must produce nothing -- but "nothing yet" and
# "nothing ever" look alike, so give the keystrokes time to be consumed
# and only then insist the queue is still empty.
run_none() {                             # $1 cmd -> queue depth after
    probe_poke "$A_ALISS" 0 0
    probe_cmd "$1"
    sleep 3
    probe_word "$A_ALISS"
}

while IFS='|' read -r row want cmd; do
    [ -z "$row" ] && continue
    printf 'row %-2s  %-26s ' "$row" "$cmd"
    if [ "$want" = "-1" ]; then
        n=$(run_none "$cmd")
        if [ "${n:-0}" -eq 0 ]; then
            pass=$((pass+1)); printf 'ok (unreachable, nothing queued)\n'
            results+=$'\n'"  ok    row $row  $cmd -> nothing (as predicted)"
        else
            got=$(probe_word "$A_AQUEU")
            fail=$((fail+1)); printf 'FAIL (queued %s)\n' "$got"
            results+=$'\n'"  FAIL  row $row  $cmd queued $got but is unreachable"
        fi
        continue
    fi

    n=$(run_one "$row" "$want" "$cmd")
    # A keystroke can be lost in a long command; retry once before
    # calling it a failure.
    [ "$n" -eq 0 ] && n=$(run_one "$row" "$want" "$cmd")
    if [ "$n" -eq 0 ]; then
        fail=$((fail+1)); printf 'FAIL (nothing queued)\n'
        results+=$'\n'"  FAIL  row $row  $cmd queued nothing, expected action $want"
        continue
    fi
    got=$(probe_word "$A_AQUEU")
    if [ "$got" = "$want" ]; then
        pass=$((pass+1)); printf 'ok (action %s)\n' "$got"
        results+=$'\n'"  ok    row $row  $cmd -> $got"
    else
        fail=$((fail+1)); printf 'FAIL (got %s want %s)\n' "$got" "$want"
        results+=$'\n'"  FAIL  row $row  $cmd -> $got, expected $want"
    fi
done <<< "$TABLE"

probe_fast on
probe_stop

echo ""
echo "==== TYPED COMMAND / ACTION ===="
echo -e "$results"
echo ""
echo "  passed $pass, failed $fail"
[ "$fail" -eq 0 ] || exit 1
exit 0
