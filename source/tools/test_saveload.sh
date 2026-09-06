#!/usr/bin/env bash
#
# test_saveload.sh -- verify both HYBER boot paths, and that a save
# file's CONTENTS actually reach the PLAYER struct.
#
# The port boots two ways depending on whether HYBER is present:
#
#   no HYBER   lc_load returns 0, g_lcldd stays 0, and main runs
#              cs_mvIn -- the move-in cutscene that seeds the resident
#   HYBER OK   lc_load reads 128 bytes into `lcp`, unpacks the door
#              bits and calls lcp_upal; g_lcldd is 1 and cs_mvIn is
#              skipped entirely
#
# The previous version of this script never ran at all: it looked for
# HYBER in $GAME_DIR/data, which does not exist -- the game opens
# "hyber" on the GEMDOS drive root -- so it exited "SETUP" every time.
# It also built without -DSKIP_COPYPROT, which parks the resident in
# cs_mvIn's `while (1) a_sleep(-1)` under any emulator here, and its
# only assertion was "did not crash" -- which a load path that silently
# did nothing would pass.
#
# So this checks the values.  The synthesised save carries distinctive
# numbers and the test reads them back out of `lcp` in memory.  Note
# that main is `g_lcldd = lc_load(); st_titl();`, and with SKIP_TITLE
# st_titl overwrites owner_name and the date, so the fields asserted
# here are deliberately ones st_titl never touches.
#
# Env: KEEP_LOG=1, HATARI=, TOS_IMG=, GAME_DIR= as in hatari_probe.sh.
# Exit: 0 both paths correct, 1 an assertion failed, 2 setup error.

set -uo pipefail
CSRC=$(cd "$(dirname "$0")/.." && pwd)
. "$CSRC/tools/hatari_probe.sh"

HYBER="$GAME_DIR/hyber"
BACKUP=$(mktemp -t hyber_bak)

# PLAYER field offsets, from include/structs.h.
OFF_CLOTHING=0x00
OFF_SPRITE=0x5a
OFF_FOOD=0x52
OFF_WATER=0x5c

# Distinctive values -- not defaults, so a stale struct cannot pass.
WANT_CLOTHING=5
WANT_SPRITE=3
WANT_FOOD=4
WANT_WATER=7

pass=0; fail=0; results=""
ok()  { pass=$((pass+1)); printf 'ok\n';             results+=$'\n'"  ok    $1"; }
bad() { fail=$((fail+1)); printf 'FAIL (%s)\n' "$2"; results+=$'\n'"  FAIL  $1 -- $2"; }

# The GEMDOS drive is shared with the developer's own play sessions, so
# put back whatever was there.
had_hyber=0
[ -f "$HYBER" ] && { had_hyber=1; cp "$HYBER" "$BACKUP"; }
restore() {
    if [ "$had_hyber" = "1" ]; then cp "$BACKUP" "$HYBER"; else rm -f "$HYBER"; fi
    rm -f "$BACKUP"
}
trap restore EXIT

write_hyber() {
    HYBER_OUT="$HYBER" python3 <<PY
import struct, os
p = bytearray(128)
def s(off, val): struct.pack_into('>h', p, off, val)
s($OFF_CLOTHING, $WANT_CLOTHING)
s(0x02, 3)     # skin_color
s(0x04, 22)    # bedtime_hour
s(0x06, 6)     # wake_hour
s(0x08, 12)    # lunch_hour
s(0x0a, 18)    # dinner_hour
s(0x0c, 1)     # personality_type
s(0x0e, 4)     # activity_level
s(0x28, 1)     # happiness
s(0x2a, 12); s(0x2c, 12); s(0x2e, 8); s(0x30, 12); s(0x32, -1)
s(0x3c, 50)    # initiative_threshold
s(0x40, 60); s(0x42, 60)      # thirst timers
s(0x46, 90); s(0x48, 90)      # hunger timers
s(0x4c, 30); s(0x4e, 30)      # bathroom timers
s($OFF_FOOD, $WANT_FOOD)
s(0x58, 0x0800)               # door_states_and_flags: food full, shut
s($OFF_SPRITE, $WANT_SPRITE)
s($OFF_WATER, $WANT_WATER)
p[0x5e:0x76] = b'SAVETEST' + b'\0' * 16
p[0x76:0x80] = b'BUDDY' + b'\0' * 5
open(os.environ['HYBER_OUT'], 'wb').write(bytes(p))
PY
}

field() {                                # $1 = offset -> decimal value
    probe_word "$(printf '%x' $(( 0x$(probe_addr _lcp) + $1 )))"
}

# Build once; both paths run the same binary.
ALCYON_CPPFLAGS="-DSKIP_TITLE=1 -DSKIP_COPYPROT=1" \
    "$CSRC/tools/alcyon_build.sh" >/dev/null 2>&1 \
    && "$CSRC/tools/alcyon_link.sh" >/dev/null 2>&1 \
    || { echo "SETUP: gated rebuild failed" >&2; exit 2; }

# ---- Path A: no HYBER, the move-in path ------------------------------
echo "==== A. no HYBER (fresh boot) ===="
rm -f "$HYBER"
NO_REBUILD=1 probe_start
echo "load base \$$(probe_base)"
printf '%-34s ' "g_lcldd == 0 (nothing loaded)"
v=$(probe_word "$(probe_addr _g_lcldd)")
[ "$v" = "0" ] && ok "path A  g_lcldd = 0" || bad "path A" "g_lcldd = $v, expected 0"
printf '%-34s ' "reached gameplay"
ok "path A  cutscene completed (introSeq cleared)"   # probe_start asserts it
probe_stop
echo ""

# ---- Path B: a valid save, values must arrive ------------------------
echo "==== B. valid HYBER (loaded save) ===="
write_hyber
[ -s "$HYBER" ] || { echo "SETUP: could not synthesise HYBER" >&2; exit 2; }
NO_REBUILD=1 probe_start
echo "load base \$$(probe_base)"

printf '%-34s ' "g_lcldd == 1 (save loaded)"
v=$(probe_word "$(probe_addr _g_lcldd)")
[ "$v" = "1" ] && ok "path B  g_lcldd = 1" || bad "path B" "g_lcldd = $v, expected 1"

# The point of the test: the file's numbers must be IN the struct.
for spec in "$OFF_WATER:$WANT_WATER:water_level" \
            "$OFF_FOOD:$WANT_FOOD:food_supply" \
            "$OFF_SPRITE:$WANT_SPRITE:character_sprite_id" \
            "$OFF_CLOTHING:$WANT_CLOTHING:clothing_color"; do
    IFS=: read -r off want name <<< "$spec"
    printf '%-34s ' "lcp.$name == $want"
    got=$(field "$off")
    [ "$got" = "$want" ] && ok "path B  lcp.$name = $got" \
                         || bad "path B lcp.$name" "got $got, expected $want"
done
probe_stop

echo ""
echo "==== HYBER SAVE/LOAD ===="
echo -e "$results"
echo ""
echo "  passed $pass, failed $fail"
[ "$fail" -eq 0 ] || exit 1
exit 0
