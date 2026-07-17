#!/usr/bin/env bash
# test_saveload.sh -- verify save/load round-trip doesn't crash.
#
# The port has two boot paths depending on whether a HYBER save file
# is present in the game data dir:
#
#   no HYBER  -> lc_load returns 0, cs_mvIn seeds player state
#                (resident at (300, 190) via the minimal cutscene)
#
#   HYBER OK  -> lc_load reads 128 bytes into PLAYER struct, unpacks
#                door states, calls lcp_upal; gameLoop sees g_lcldd=1
#                and positions player near POS_TOP_STUDY_DOOR
#
# Both paths must boot without crashing.  This test runs each in turn
# and compares the resulting bus-error / crash-detector output.  A
# synthesised HYBER with sane defaults is generated in Python.
#
# Exit codes:
#   0 both paths boot clean
#   1 either path crashed (see log for details)
#   2 setup error

set -uo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
GAME=${GAME_DIR:-$HOME/hatari-c/GAME}
DATA_DIR=$GAME/data
HYBER=$DATA_DIR/HYBER   # Alcyon Fopen is case-insensitive per Atari conv
BACKUP=/tmp/hyber.saveload_test.bak
LOG=/tmp/lcp_saveload.log

if [ ! -d "$DATA_DIR" ]; then
    echo "SETUP: $DATA_DIR missing" >&2
    exit 2
fi

# Preserve any existing hyber the user has.
had_hyber=0
if [ -f "$HYBER" ]; then
    had_hyber=1
    cp "$HYBER" "$BACKUP"
fi

restore() {
    if [ "$had_hyber" = "1" ]; then
        mv "$BACKUP" "$HYBER"
    else
        rm -f "$HYBER"
    fi
}
trap restore EXIT

# ---- Path A: no HYBER -----------------------------------------------
rm -f "$HYBER"
echo "==== A. no HYBER (fresh-boot path) ===="
LOG="$LOG" VBLS=1500 "$CSRC/tools/run_hatari.sh"
a=$?
echo ""

# ---- Path B: valid HYBER --------------------------------------------
# Synthesise a 128-byte PLAYER struct with sane defaults.  Field offsets
# from include/structs.h.  All shorts big-endian (68k byte order).
echo "==== B. valid HYBER (loaded save path) ===="
HYBER_OUT="$HYBER" python3 <<'PY'
import struct, os
out = os.environ['HYBER_OUT']
p = bytearray(128)
def put_s(off, val):
    struct.pack_into('>h', p, off, val)

put_s(0x00, 5)      # clothing_color
put_s(0x02, 3)      # skin_color
put_s(0x04, 22)     # bedtime_hour
put_s(0x06, 6)      # wake_hour
put_s(0x08, 12)     # lunch_hour
put_s(0x0a, 18)     # dinner_hour
put_s(0x0c, 1)      # personality_type
put_s(0x0e, 4)      # activity_level
# 0x10..0x27 reserved (zero)
put_s(0x28, 1)      # happiness (MOOD_CONTENT)
put_s(0x2a, 12)     # happiness_initial_countdown
put_s(0x2c, 12)     # happiness_duration_happy
put_s(0x2e, 8)      # happiness_duration_content
put_s(0x30, 12)     # happiness_duration_active
put_s(0x32, -1)     # happiness_direction
put_s(0x34, 0)      # sickness_level
put_s(0x36, 0)      # sickness_countdown
put_s(0x38, 0)      # sickness_direction
put_s(0x3a, 0)      # is_sleeping
put_s(0x3c, 50)     # initiative_threshold
put_s(0x3e, 0)      # thirst_level
put_s(0x40, 60)     # thirst_timer_max
put_s(0x42, 60)     # thirst_timer
put_s(0x44, 0)      # hunger_level
put_s(0x46, 90)     # hunger_timer_max
put_s(0x48, 90)     # hunger_timer
put_s(0x4a, 0)      # bathroom_need
put_s(0x4c, 30)     # bathroom_timer_max
put_s(0x4e, 30)     # bathroom_timer
# 0x50 reserved
put_s(0x52, 4)      # food_supply
put_s(0x54, 0)      # record_playing
put_s(0x56, 0)      # tv_on
# door_states_and_flags @ 0x58: init full-food, everything closed.
put_s(0x58, 0x0800) # DSF_INIT_FOOD_FULL
put_s(0x5a, 3)      # character_sprite_id (PE3.LCP)
put_s(0x5c, 7)      # water_level
name = b'PLAYER\x00' + b'\x00' * 17    # 24 bytes @ 0x5e
p[0x5e:0x76] = name
cname = b'BUDDY\x00\x00\x00\x00\x00'    # 10 bytes @ 0x76
p[0x76:0x80] = cname
open(out, 'wb').write(bytes(p))
print(f'wrote {len(p)} bytes -> {out}')
PY

LOG="$LOG" VBLS=1500 "$CSRC/tools/run_hatari.sh"
b=$?
echo ""

# ---- Verdict ---------------------------------------------------------
echo "==== SAVE/LOAD VERDICT ===="
echo "  path A (no HYBER):     exit=$a"
echo "  path B (valid HYBER):  exit=$b"
if [ "$a" = "0" ] && [ "$b" = "0" ]; then
    echo "  both paths boot clean"
    exit 0
fi
echo "  at least one path crashed"
exit 1
