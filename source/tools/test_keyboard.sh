#!/usr/bin/env bash
#
# *** DOES NOT RUN.  TEST_KEY is no longer implemented anywhere in
# source/ -- the in-game keyboard harness it switched on was removed
# during the LCP_STX restructuring, so the flag below compiles to
# nothing and this script exercises no hook.  SKIP_TITLE and
# SKIP_COPYPROT here are current and correct; re-instating the
# keyboard hook is what is missing. ***
#
# test_keyboard.sh -- exercise every Ctrl-letter keyboard dispatch
# path by invoking deal_kc(keycode) at startup, then running the
# smoke test to look for crashes.
#
# Same pattern as tools/test_actions.sh:
#   -DTEST_KEY=$keycode in cs_mvIn calls deal_kc directly, gameLoop
#   then runs normally.
#
# Ctrl+A..W keycodes are ASCII 1..23.  We skip printable chars (>=
# 0x20) because they hit prCh -> v_gtext which is a known open crash
# (see git log for the $fd330c investigation).  Ctrl+M submits the
# command buffer -- safe with an empty buffer.
#
# Env:
#   VBLS       run length per test  (default 1500 -- below v_gtext)
#
# Exit:
#   0 all keys clean
#   1 at least one crashed (see log)
#   2 setup error

set -uo pipefail
CSRC=$(cd "$(dirname "$0")/.." && pwd)
VBLS=${VBLS:-1500}

# Keycode -> friendly-name pairs.  Values from include/enums.h
# KEY_CTRL_*_* macros (all ASCII 0x01..0x17).
tests=(
    "1=CTRL_A_ALARM"
    "2=CTRL_B_BOOK"
    "3=CTRL_C_CALL"
    "4=CTRL_D_DOGFOOD"
    "6=CTRL_F_FOOD"
    "13=CTRL_M_ENTER"
    "16=CTRL_P_PATTING"
    "18=CTRL_R_RECORD"
    "23=CTRL_W_WATER"
)

fail=0
results=""
for entry in "${tests[@]}"; do
    id=${entry%=*}
    name=${entry#*=}
    printf "==== %-16s (code=%3s)  " "$name" "$id"

    ALCYON_CPPFLAGS="-DTEST_KEY=$id -DSKIP_TITLE=1 -DSKIP_MIDI=1 -DSKIP_COPYPROT=1" \
        FILES=init.c "$CSRC/tools/alcyon_build.sh" > /dev/null 2>&1 || {
        echo "BUILD-FAIL"; fail=1; results+="\n  $name  BUILD-FAIL"; continue
    }
    "$CSRC/tools/alcyon_link.sh" > /dev/null 2>&1 || {
        echo "LINK-FAIL"; fail=1; results+="\n  $name  LINK-FAIL"; continue
    }
    VBLS=$VBLS "$CSRC/tools/run_hatari.sh" > /tmp/key_$id.log 2>&1
    rc=$?
    if [ "$rc" = "0" ]; then
        echo "clean"
        results+="\n  $name  clean"
    else
        n_be=$(grep -c 'Bus Error' /tmp/key_$id.log)
        fpc=$(grep 'Bus Error' /tmp/key_$id.log | head -1 | sed 's/.*PC=\$\([0-9a-f]*\).*/\1/')
        echo "CRASH  ($n_be errs, first PC=\$$fpc)"
        results+="\n  $name  CRASH pc=\$$fpc"
        fail=1
    fi
done

echo ""
echo "==== SUMMARY ===="
echo -e "$results"

# Rebuild clean.
FILES=init.c "$CSRC/tools/alcyon_build.sh" > /dev/null 2>&1
"$CSRC/tools/alcyon_link.sh" > /dev/null 2>&1

exit $fail
