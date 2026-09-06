#!/usr/bin/env bash
# hatari_probe.sh -- shared harness for the runtime regression scripts.
# Source it; it is not runnable on its own.
#
# What it gives you: a headless Hatari running the gated build, with a
# command FIFO for injecting keystrokes and reading the game's own
# globals out of memory.  That is what makes an assertion possible --
# "the key did something" is checked against the variable the handler
# writes, not against a screenshot.
#
#   probe_start            build (unless NO_REBUILD), boot, reach gameplay
#   probe_addr  SYM        runtime address of a port symbol, hex, no $
#   probe_mem   ADDR N     N bytes at ADDR as "xx xx xx"
#   probe_word  ADDR       one big-endian 16-bit word, decimal
#   probe_key   KEY        inject a keypress (char or ST scancode)
#   probe_ctrl  LETTER     inject Ctrl+<letter>
#   probe_type  "TEXT"     type a string, one keypress per character
#   probe_stop             shut the emulator down
#
# THE LOAD BASE IS DISCOVERED, NOT ASSUMED.  It differs by launch path
# (--auto and the MCP's run_program disagree by 0x104), and a wrong
# base does not fail -- it reads plausible-looking garbage, which has
# already cost this project one wrong conclusion.  So probe_start
# hunts for bm32or's 1,2,4,8,16 longs in a window around where they
# ought to be and derives the base from where they actually are.  If
# it cannot find them it aborts rather than guess.

HATARI=${HATARI:-$HOME/Downloads/Hatari/hatari/build/src/hatari}
TOS_IMG=${TOS_IMG:-/Users/sarnau/Desktop/Retro/Atari ST/Atari TOS Images/TOS104US.ROM}
GAME_DIR=${GAME_DIR:-$HOME/Hatari_C/hatari-c/GAME}
BOOT_VBLS=${BOOT_VBLS:-20000}      # enough for the move-in cutscene

_PROBE_TMP=""
_PROBE_LOG=""
_PROBE_FIFO=""
_PROBE_BASE=""
_PROBE_SYMS=""

probe_die() { echo "SETUP: $*" >&2; probe_stop 2>/dev/null; exit 2; }

# --- symbol table -----------------------------------------------------
# lcp_sym.68k carries 8-char TRUNCATED linkage names with a leading
# underscore, so ask for what the linker actually emitted (_lcp_wat,
# not _lcp_watr).
_probe_load_syms() {
    _PROBE_SYMS=$(python3 - "$CSRC/build/alcyon/lcp_sym.68k" <<'PY'
import struct, sys
d = open(sys.argv[1], 'rb').read()
magic, t, dd, b, s = struct.unpack('>HIIII', d[:18])
off, end = 0x1C + t + dd, 0x1C + t + dd + s
while off < end:
    name = d[off:off+8].rstrip(b'\0').decode('ascii', 'replace')
    typ, val = struct.unpack('>HI', d[off+8:off+14])
    off += 14
    print('%s %d' % (name, val))
PY
) || probe_die "cannot read build/alcyon/lcp_sym.68k"
}

_probe_link_off() {                      # $1 = symbol -> link offset
    echo "$_PROBE_SYMS" | awk -v s="$1" '$1==s {print $2; found=1; exit}
                                         END {if (!found) print "?"}'
}

probe_addr() {                           # $1 = symbol -> runtime hex
    local off; off=$(_probe_link_off "$1")
    [ "$off" = "?" ] && probe_die "symbol $1 not in lcp_sym.68k"
    printf '%x' $(( off + _PROBE_BASE ))
}

# --- talking to the emulator -----------------------------------------
_probe_send() { printf '%s\n' "$*" > "$_PROBE_FIFO"; }

# Send a debugger command and return the lines it printed.  The reply
# is found by remembering how long the log was beforehand, which is
# the only ordering guarantee available over a one-way FIFO.
_probe_debug() {
    local before after
    before=$(wc -l < "$_PROBE_LOG")
    _probe_send "hatari-debug $*"
    for _ in $(seq 1 40); do
        after=$(wc -l < "$_PROBE_LOG")
        [ "$after" -gt "$before" ] && break
        sleep 0.05
    done
    tail -n +$((before + 1)) "$_PROBE_LOG"
}

# Match the reply by the ADDRESS it prints, not merely by "the log
# grew".  The FIFO is one-way with no request ids, so a late reply
# would otherwise be read as the answer to the next question -- which
# silently returns the PREVIOUS value and looks exactly like an
# off-by-one bug in the thing under test.
probe_mem() {                            # $1 = hex addr, $2 = count
    local want line before
    want=$(printf '%08X' "0x$1")
    before=$(wc -l < "$_PROBE_LOG")
    _probe_send "hatari-debug m \$$1 $2"
    for _ in $(seq 1 60); do
        line=$(tail -n +$((before + 1)) "$_PROBE_LOG" | grep -E "^$want:" | tail -1)
        [ -n "$line" ] && break
        sleep 0.05
    done
    [ -z "$line" ] && { echo ""; return 1; }
    echo "$line" | sed -E "s/^$want: //; s/  .*$//" | sed 's/ *$//'
}

probe_word() {                           # $1 = hex addr -> decimal
    local b; b=$(probe_mem "$1" 2)
    local hi lo; hi=${b%% *}; lo=${b##* }
    echo $(( 0x$hi * 256 + 0x$lo ))
}

# hatari-event takes EITHER one alphanumeric character, which is typed
# as itself, OR a number -- and a number is an ST SCANCODE, not ASCII.
# So anything that is not a letter or digit has to go in as a scancode,
# and Ctrl combinations have to hold the real Control key down.
SC_CTRL=29
SC_SPACE=57
SC_RETURN=28
SC_BACKSPACE=14
SC_LEFT=75

probe_key()  { _probe_send "hatari-event keypress $1"; sleep 0.03; }
probe_enter() { probe_key $SC_RETURN; sleep 0.15; }

probe_ctrl() {                           # $1 = letter -> Ctrl+<letter>
    _probe_send "hatari-event keydown $SC_CTRL"
    _probe_send "hatari-event keypress $(echo "$1" | tr 'A-Z' 'a-z')"
    _probe_send "hatari-event keyup $SC_CTRL"
    sleep 0.1
}

probe_type() {                           # $1 = text (no Return)
    local i c
    for (( i=0; i<${#1}; i++ )); do
        c=${1:i:1}
        case "$c" in
            [A-Za-z0-9]) probe_key "$c" ;;
            ' ')         probe_key $SC_SPACE ;;
            *)           probe_die "probe_type cannot send '$c'" ;;
        esac
    done
}

probe_cmd() { probe_type "$1"; probe_enter; }   # type a command + Return

# --- lifecycle --------------------------------------------------------
probe_start() {
    command -v python3 >/dev/null || probe_die "python3 missing"
    [ -x "$HATARI" ]   || probe_die "hatari not at $HATARI (set HATARI=)"
    [ -f "$TOS_IMG" ]  || probe_die "TOS ROM missing at $TOS_IMG"

    if [ -z "${NO_REBUILD:-}" ]; then
        ALCYON_CPPFLAGS="-DSKIP_TITLE=1 -DSKIP_COPYPROT=1" \
            "$CSRC/tools/alcyon_build.sh" >/dev/null 2>&1 \
            && "$CSRC/tools/alcyon_link.sh" >/dev/null 2>&1 \
            || probe_die "gated rebuild failed"
    fi
    [ -f "$CSRC/build/alcyon/LCP.PRG" ] || probe_die "LCP.PRG missing"
    _probe_load_syms

    _PROBE_TMP=$(mktemp -d -t lcp_probe)
    _PROBE_LOG=$_PROBE_TMP/hatari.log
    _PROBE_FIFO=$_PROBE_TMP/cmd.fifo
    rm -f "$_PROBE_FIFO"                 # Hatari creates it itself

    cp -f "$CSRC/build/alcyon/LCP.PRG" "$GAME_DIR/LCP.PRG" \
        || probe_die "cannot stage LCP.PRG into $GAME_DIR"
    rm -f "$GAME_DIR/LCP.SAV"            # force the cs_mvIn path
    pkill -x hatari 2>/dev/null; sleep 1

    "$HATARI" --harddrive "$GAME_DIR" --tos "$TOS_IMG" \
              --machine st --cpulevel 0 --cpuclock 8 --memsize 1 \
              --fast-forward on --cmd-fifo "$_PROBE_FIFO" \
              --auto 'C:\LCP.PRG' > "$_PROBE_LOG" 2>&1 &
    for _ in $(seq 1 60); do [ -p "$_PROBE_FIFO" ] && break; sleep 0.25; done
    [ -p "$_PROBE_FIFO" ] || probe_die "Hatari never created the command FIFO"

    sleep 8                              # TOS boot + program load
    _probe_find_base
    probe_gameplay_ready
}

# Locate bm32or's 1,2,4,8,16 longs and derive the base from where they
# really are.  Never assume: a wrong base reads garbage silently.
_probe_find_base() {
    local link want lo dump
    link=$(_probe_link_off _bm32or)
    [ "$link" = "?" ] && probe_die "_bm32or missing from lcp_sym.68k"
    want="00 00 00 01 00 00 00 02 00 00 00 04 00 00 00 08"
    for lo in 0x12492 0x12596; do        # known launch-path bases first
        _PROBE_BASE=$lo
        dump=$(probe_mem "$(printf '%x' $((lo + link)))" 16)
        if [ "$dump" = "$want" ]; then return 0; fi
    done
    # Widen: dump a window and search for the signature.
    local start; start=$(( 0x12000 + link ))
    dump=$(_probe_debug "m \$$(printf '%x' $start) 2048")
    local hit
    hit=$(echo "$dump" | grep -E '^[0-9A-F]{8}: 00 00 00 01 00 00 00 02' | head -1)
    [ -z "$hit" ] && probe_die "cannot locate bm32or; load base undetermined"
    _PROBE_BASE=$(( 0x${hit%%:*} - link ))
    return 0
}

# The cutscene must be over before typed input is dispatched at all --
# tick.c gates the keyboard on `introSeq == NO`.
#
# How long that takes depends entirely on whether a save was loaded.
# With a HYBER present lc_load sets g_lcldd and main SKIPS cs_mvIn, so
# introSeq is clear almost at once.  Without one the full move-in
# cutscene runs -- doorbell, kitchen, sink, dresser, bathroom, suitcase
# -- and that is ~60 s of wall time even fast-forwarded.  Wait for the
# slow case; the fast one returns on the first poll.
probe_gameplay_ready() {
    local a v
    a=$(probe_addr _introSe)
    for _ in $(seq 1 ${READY_TIMEOUT:-180}); do
        v=$(probe_word "$a")
        [ "$v" = "0" ] && return 0
        sleep 1
    done
    probe_die "introSeq never cleared after ${READY_TIMEOUT:-180}s -- gameplay not reached"
}

probe_base() { printf '%x' "$_PROBE_BASE"; }

# --- breakpoints ------------------------------------------------------
# Some of what these scripts check is TRANSIENT: alarm_p is set by the
# key handler and cleared again by the game a frame or two later, so a
# direct read races and usually loses.  A value-change breakpoint
# cannot be sampled past -- it fires on both the set and the clear.
probe_mark()  { wc -l < "$_PROBE_LOG"; }
probe_since() { tail -n +$(( $1 + 1 )) "$_PROBE_LOG"; }

probe_bp_pc()      { _probe_send "hatari-debug b pc = \$$1 :trace"; sleep 0.2; }
probe_bp_changed() { _probe_send "hatari-debug b (\$$1).w ! (\$$1).w :trace"; sleep 0.2; }
probe_bp_clear()   { _probe_send "hatari-debug b -all"; sleep 0.2; }

# Count breakpoint hits recorded since a probe_mark.  Hatari prints one
# "condition(s) matched" line per hit.
probe_hits() { probe_since "$1" | grep -c 'condition(s) matched' || true; }

# Write bytes.  Used to satisfy a guard the AI would otherwise have to
# reach on its own -- Ctrl-P is gated on pat_ok, which only a_calld
# sets and no typed command reaches.
probe_poke() { local a=$1; shift; _probe_send "hatari-debug w \$$a $*"; sleep 0.1; }

# Fast-forward compresses wall time but NOT emulated time, so with it on
# a few hundred emulated frames pass between two commands here.  That is
# fine for booting and fatal for measuring anything short-lived -- the
# text buffer times out (tx_sctm is 160 ticks) and the action queue
# drains.  Turn it off around a measurement.
probe_fast() { _probe_send "hatari-option --fast-forward $1"; sleep 0.3; }

probe_stop() {
    [ -n "$_PROBE_FIFO" ] && [ -p "$_PROBE_FIFO" ] && _probe_send "hatari-shortcut quit"
    sleep 1
    pkill -x hatari 2>/dev/null
    [ -n "${KEEP_LOG:-}" ] && echo "log: $_PROBE_LOG" >&2
    [ -z "${KEEP_LOG:-}" ] && [ -n "$_PROBE_TMP" ] && rm -rf "$_PROBE_TMP"
    return 0
}
