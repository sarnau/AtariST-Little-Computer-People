#!/usr/bin/env bash
# test_actions.sh -- exercise every ported AI action / delivery event
# at runtime by injecting each as a startup event via cs_mvIn.
#
# cs_mvIn has a guarded #ifdef TEST_ACTIONS block that pushes one
# event into the g_trel FIFO before returning; gameLoop's AI
# dispatcher pops and dispatches it, giving us end-to-end coverage
# of that action.  We loop through every action ID, build LCP.PRG
# with -D TEST_ACTIONS=$id, and run the smoke test.  Any bus error
# is attributed to that specific action.
#
# Env:
#   VBLS       run length per test  (default 1500 -- below v_gtext)
#
# Exit:
#   0 all tested actions clean
#   1 at least one crashed (see log)
#   2 setup error

set -uo pipefail
CSRC=$(cd "$(dirname "$0")/.." && pwd)
VBLS=${VBLS:-1500}

# ACTION_ID enum values from include/enums.h.
# name -> id.  Keep list under ~15 so total run is manageable.
tests=(
    "ACTION_HELLO=18"
    "ACTION_YAWN_AND_STRETCH=41"
    "ACTION_NOD_HEAD=24"
    "ACTION_PET_DOG=30"
    "ACTION_CALL_DOG=2"
    "ACTION_WANDER_IDLY=45"
    "ACTION_PEEK_AROUND=29"
    "ACTION_PACE_NERVOUSLY=28"
    "ACTION_DANCE=5"
    "ACTION_READ_NEWSPAPER=35"
    "ACTION_LISTEN_SONG=23"
    "ACTION_TIDY_HOUSE=40"
    "ACTION_EVENT_PHONE_CALL=11"
    "ACTION_EVENT_FOOD_DELIVERY=10"
    "ACTION_EVENT_BOOK_DELIVERY=8"
)

fail=0
results=""
for entry in "${tests[@]}"; do
    name=${entry%=*}
    id=${entry#*=}
    printf "==== %-30s (id=%s)  " "$name" "$id"

    # Add -DTEST_ACTIONS=$id via cp68 flags temporarily.
    # alcyon_build.sh doesn't take custom flags out of the box, so
    # we set ALCYON_CPPFLAGS via the environment.
    ALCYON_CPPFLAGS="-DTEST_ACTIONS=$id" \
        FILES=init.c "$CSRC/tools/alcyon_build.sh" > /dev/null 2>&1 || {
        echo "BUILD-FAIL"; fail=1; results+="\n  $name  BUILD-FAIL"; continue
    }
    "$CSRC/tools/alcyon_link.sh" > /dev/null 2>&1 || {
        echo "LINK-FAIL"; fail=1; results+="\n  $name  LINK-FAIL"; continue
    }
    VBLS=$VBLS "$CSRC/tools/run_hatari.sh" > /tmp/action_$id.log 2>&1
    rc=$?
    if [ "$rc" = "0" ]; then
        echo "clean"
        results+="\n  $name  clean"
    else
        n_be=$(grep -c 'Bus Error' /tmp/action_$id.log)
        fpc=$(grep 'Bus Error' /tmp/action_$id.log | head -1 | sed 's/.*PC=\$\([0-9a-f]*\).*/\1/')
        echo "CRASH  ($n_be errs, first PC=\$$fpc)"
        results+="\n  $name  CRASH pc=\$$fpc"
        fail=1
    fi
done

echo ""
echo "==== SUMMARY ===="
echo -e "$results"

# Rebuild clean (without TEST_ACTIONS).
FILES=init.c "$CSRC/tools/alcyon_build.sh" > /dev/null 2>&1
"$CSRC/tools/alcyon_link.sh" > /dev/null 2>&1

exit $fail
