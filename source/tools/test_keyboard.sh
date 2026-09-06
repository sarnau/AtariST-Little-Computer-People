#!/usr/bin/env bash
#
# test_keyboard.sh -- check every deal_kc dispatch path by injecting the
# real keystroke and asserting on the global the handler writes.
#
# This replaces a version that DID NOT RUN: it built with -DTEST_KEY=n
# to switch on an in-game harness that called deal_kc directly, and
# that harness was removed during the LCP_STX restructuring, so the
# flag compiled to nothing and the script exercised no hook while still
# reporting success.  Nothing is gated into the port now -- the keys go
# in through the IKBD exactly as a player's would, and the check is the
# variable the handler actually touches.
#
# Verified by STATE, not by animation:
#
#   Ctrl-W  water      lcp_watr increments (clamped at 10)
#   Ctrl-A  alarm      alarm_p changes -- SET then CLEARED, so this is
#                      caught with a value-change breakpoint; a direct
#                      read races the game and usually loses
#   Ctrl-B  book       putEv() entered
#   Ctrl-C  phone      putEv() entered
#   Ctrl-D  dog food   putEv() entered
#   Ctrl-F  food       putEv() entered
#   Ctrl-R  record     putEv() entered
#   Ctrl-P  pat dog    g_ptdoa changes.  Guarded by dg_petok, which only
#                      a_calld sets and NO typed command reaches, so the
#                      guard is forced from the debugger -- otherwise
#                      this key is untestable without waiting on the AI.
#   Ctrl-M  Return     g_aliss grows (a command is submitted)
#   8       erase      g_cdibp decrements.  Reached from BOTH Backspace
#                      and the cursor-LEFT arrow: getKey maps scancode
#                      0x4b to 8 and Backspace is ASCII 8 already.
#
# Env: NO_REBUILD=1 reuse the current gated build; KEEP_LOG=1 keep the
#      Hatari log; HATARI=, TOS_IMG=, GAME_DIR= as in hatari_probe.sh.
#
# Exit: 0 all keys behaved, 1 at least one did not, 2 setup error.

set -uo pipefail
CSRC=$(cd "$(dirname "$0")/.." && pwd)
. "$CSRC/tools/hatari_probe.sh"

pass=0; fail=0; results=""

ok()   { pass=$((pass+1)); printf 'ok\n';                results+=$'\n'"  ok    $1"; }
bad()  { fail=$((fail+1)); printf 'FAIL (%s)\n' "$2";    results+=$'\n'"  FAIL  $1 -- $2"; }

probe_start

A_WATR=$(probe_addr _lcp_wat)
A_ALRM=$(probe_addr _alarm_p)
A_PUTEV=$(probe_addr _putEv)
A_PTDOA=$(probe_addr _g_ptdoa)
A_PETOK=$(probe_addr _dg_peto)
A_ALISS=$(probe_addr _g_aliss)
A_CDIBP=$(probe_addr _g_cdibp)

echo "load base \$$(probe_base)"
echo ""

# ---- Ctrl-W: the one handler with a durable counter ------------------
printf '%-22s ' "Ctrl-W  water"
before=$(probe_word "$A_WATR")
probe_ctrl W; probe_ctrl W; probe_ctrl W; sleep 0.5
after=$(probe_word "$A_WATR")
if [ "$after" -gt "$before" ]; then ok "Ctrl-W  lcp_watr $before -> $after"
else bad "Ctrl-W" "lcp_watr stayed $before"; fi

# ---- the five that queue an event ------------------------------------
for pair in "B book" "C phone" "D dogfood" "F food" "R record"; do
    set -- $pair
    printf '%-22s ' "Ctrl-$1  $2"
    probe_bp_clear; probe_bp_pc "$A_PUTEV"
    M=$(probe_mark); probe_ctrl "$1"; sleep 0.6
    n=$(probe_hits "$M")
    if [ "$n" -ge 1 ]; then ok "Ctrl-$1  putEv entered"
    else bad "Ctrl-$1" "putEv never entered"; fi
done
probe_bp_clear

# ---- Ctrl-A: transient, so watch the cell instead of reading it ------
printf '%-22s ' "Ctrl-A  alarm"
probe_bp_changed "$A_ALRM"
M=$(probe_mark); probe_ctrl A; sleep 0.6
n=$(probe_hits "$M")
if [ "$n" -ge 1 ]; then ok "Ctrl-A  alarm_p changed ($n)"
else bad "Ctrl-A" "alarm_p never changed"; fi
probe_bp_clear

# ---- Ctrl-P: force the guard the AI would have to satisfy ------------
printf '%-22s ' "Ctrl-P  pat dog"
probe_poke "$A_PETOK" 0 1                  # dg_petok = YES
probe_bp_changed "$A_PTDOA"
M=$(probe_mark); probe_ctrl P; sleep 0.6
n=$(probe_hits "$M")
if [ "$n" -ge 1 ]; then ok "Ctrl-P  g_ptdoa changed ($n)"
else bad "Ctrl-P" "g_ptdoa never changed (dg_petok guard?)"; fi
probe_bp_clear

# ---- Ctrl-M: Return submits the command buffer -----------------------
printf '%-22s ' "Ctrl-M  submit"
before=$(probe_word "$A_ALISS")
probe_cmd "DRINK WATER"; sleep 0.4
after=$(probe_word "$A_ALISS")
if [ "$after" -gt "$before" ]; then ok "Ctrl-M  g_aliss $before -> $after"
else bad "Ctrl-M" "g_aliss stayed $before -- nothing submitted"; fi

# ---- key 8: from Backspace AND from the cursor-left arrow ------------
for pair in "$SC_BACKSPACE Backspace" "$SC_LEFT cursor-left"; do
    set -- $pair
    printf '%-22s ' "erase   $2"
    probe_type "ABCDE"; sleep 0.2
    before=$(probe_word "$A_CDIBP")
    probe_key "$1"; sleep 0.3
    after=$(probe_word "$A_CDIBP")
    if [ "$before" -gt 0 ] && [ "$after" -eq $((before - 1)) ]; then
        ok "$2  g_cdibp $before -> $after"
    else
        bad "$2" "g_cdibp $before -> $after (expected one less)"
    fi
    probe_key "$SC_RETURN"; sleep 0.2      # clear the buffer
done

probe_stop

echo ""
echo "==== KEYBOARD DISPATCH ===="
echo -e "$results"
echo ""
echo "  passed $pass, failed $fail"
[ "$fail" -eq 0 ] || exit 1
exit 0
