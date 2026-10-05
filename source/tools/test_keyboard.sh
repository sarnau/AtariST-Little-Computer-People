#!/usr/bin/env bash
#
# test_keyboard.sh -- check every handleKey dispatch path by injecting the
# real keystroke and asserting on the global the handler writes.
#
# This replaces a version that DID NOT RUN: it built with -DTEST_KEY=n
# to switch on an in-game harness that called handleKey directly, and
# that harness was removed during the LCP_STX restructuring, so the
# flag compiled to nothing and the script exercised no hook while still
# reporting success.  Nothing is gated into the port now -- the keys go
# in through the IKBD exactly as a player's would, and the check is the
# variable the handler actually touches.
#
# Verified by STATE, not by animation:
#
#   Ctrl-W  water      waterLevel increments (clamped at 10)
#   Ctrl-A  alarm      alarmRinging changes -- SET then CLEARED, so this is
#                      caught with a value-change breakpoint; a direct
#                      read races the game and usually loses
#   Ctrl-B  book       queueEvent() entered
#   Ctrl-C  phone      queueEvent() entered
#   Ctrl-D  dog food   queueEvent() entered
#   Ctrl-F  food       queueEvent() entered
#   Ctrl-R  record     queueEvent() entered
#   Ctrl-P  pat LCP    patActive changes.  This pats the RESIDENT on the
#                      head, not the dog: the handler sets
#                      lcp.happiness = MOOD_HAPPY, and the animation
#                      cycles SPRITE_PET_HAND_1..6 -- the player's hand
#                      -- at a fixed (192,165), which is where the
#                      phone sits (tick.c draws it at 190,168).  It is
#                      gated on patAllowed, set only by callDog, which
#                      walks the resident to position 43 (x=220, the
#                      armchair by the phone) and crouches.  No typed
#                      command reaches callDog, so the guard is forced
#                      from the debugger.
#   Ctrl-M  Return     queueCount grows (a command is submitted)
#   8       erase      typedCursor decrements.  Reached from BOTH Backspace
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

A_WATR=$(probe_addr _waterLe)
A_ALRM=$(probe_addr _alarmRi)
A_PUTEV=$(probe_addr _queueEv)
A_PTDOA=$(probe_addr _patActi)
A_PETOK=$(probe_addr _patAllo)
A_ALISS=$(probe_addr _queueCo)
A_CDIBP=$(probe_addr _typedCu)

echo "load base \$$(probe_base)"
echo ""

# ---- Ctrl-W: durable counter, but it CLAMPS ---------------------------
# handleKey returns immediately when the tank is already full:
#   if (waterLevel == 10) return;
# and earlier runs of this very script leave it at 10, so asserting an
# increase without emptying it first fails on the second run of the day.
# Empty it, then three presses must add three.
printf '%-22s ' "Ctrl-W  water"
probe_poke "$A_WATR" 0 0
before=$(probe_word "$A_WATR")
probe_ctrl W; probe_ctrl W; probe_ctrl W; sleep 0.5
after=$(probe_word "$A_WATR")
if [ "$after" -eq $((before + 3)) ]; then ok "Ctrl-W  waterLevel $before -> $after"
else bad "Ctrl-W" "waterLevel $before -> $after, expected $((before + 3))"; fi

# ---- Ctrl-F is CONDITIONAL, so make its condition true ---------------
# handleKey returns without queuing when the food cupboard reads full:
#   if (((lcp.door_states_and_flags >> 9) & 7) == 4) { pantryFull = YES; return; }
# Whether that holds depends on the save that happens to be on the
# drive, so this asserted nothing stable until the field was forced.
# Clear bits 9..11 and the delivery path is the one under test.
printf '%-22s ' "Ctrl-F  food"
A_DSF=$(printf '%x' $(( 0x$(probe_addr _residen) + 0x58 )))
dsf=$(probe_word "$A_DSF")
newdsf=$(( dsf & ~0x0E00 ))
probe_poke "$A_DSF" $(( (newdsf >> 8) & 0xff )) $(( newdsf & 0xff ))
probe_bp_clear; probe_bp_pc "$A_PUTEV"
M=$(probe_mark); probe_ctrl F; sleep 0.6
n=$(probe_hits "$M")
if [ "$n" -ge 1 ]; then ok "Ctrl-F  queueEvent entered (food field forced < 4)"
else bad "Ctrl-F" "queueEvent never entered even with the food field cleared"; fi
probe_bp_clear

# ---- the four unconditional event keys -------------------------------
for pair in "B book" "C phone" "D dogfood" "R record"; do
    set -- $pair
    printf '%-22s ' "Ctrl-$1  $2"
    probe_bp_clear; probe_bp_pc "$A_PUTEV"
    M=$(probe_mark); probe_ctrl "$1"; sleep 0.6
    n=$(probe_hits "$M")
    if [ "$n" -ge 1 ]; then ok "Ctrl-$1  queueEvent entered"
    else bad "Ctrl-$1" "queueEvent never entered"; fi
done
probe_bp_clear

# ---- Ctrl-A: transient, so watch the cell instead of reading it ------
printf '%-22s ' "Ctrl-A  alarm"
probe_bp_changed "$A_ALRM"
M=$(probe_mark); probe_ctrl A; sleep 0.6
n=$(probe_hits "$M")
if [ "$n" -ge 1 ]; then ok "Ctrl-A  alarmRinging changed ($n)"
else bad "Ctrl-A" "alarmRinging never changed"; fi
probe_bp_clear

# ---- Ctrl-P: force the guard the AI would have to satisfy ------------
printf '%-22s ' "Ctrl-P  pat LCP"
probe_poke "$A_PETOK" 0 1                  # patAllowed = YES
probe_bp_changed "$A_PTDOA"
M=$(probe_mark); probe_ctrl P; sleep 0.6
n=$(probe_hits "$M")
if [ "$n" -ge 1 ]; then ok "Ctrl-P  patActive changed ($n)"
else bad "Ctrl-P" "patActive never changed (patAllowed guard?)"; fi
probe_bp_clear

# ---- Ctrl-M: Return submits the command buffer -----------------------
printf '%-22s ' "Ctrl-M  submit"
before=$(probe_word "$A_ALISS")
probe_cmd "DRINK WATER"; sleep 0.4
after=$(probe_word "$A_ALISS")
if [ "$after" -gt "$before" ]; then ok "Ctrl-M  queueCount $before -> $after"
else bad "Ctrl-M" "queueCount stayed $before -- nothing submitted"; fi

# ---- key 8: from Backspace AND from the cursor-left arrow ------------
for pair in "$SC_BACKSPACE Backspace" "$SC_LEFT cursor-left"; do
    set -- $pair
    printf '%-22s ' "erase   $2"
    probe_type "ABCDE"; sleep 0.2
    before=$(probe_word "$A_CDIBP")
    probe_key "$1"; sleep 0.3
    after=$(probe_word "$A_CDIBP")
    if [ "$before" -gt 0 ] && [ "$after" -eq $((before - 1)) ]; then
        ok "$2  typedCursor $before -> $after"
    else
        bad "$2" "typedCursor $before -> $after (expected one less)"
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
