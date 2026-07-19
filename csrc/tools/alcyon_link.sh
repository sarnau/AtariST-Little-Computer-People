#!/usr/bin/env bash
# alcyon_link.sh -- link with the ATARI DEVELOPER KIT toolchain.
#
# This uses the actual 1985/86 Atari DK libraries and startup that
# shipping-era ST games (including LCP) were built against, instead
# of the newer TOOLS/LIB replacement runtime.
#
# Composition (matches DISK_2/LINKER/LINKAP.BAT):
#   gemstart.o        DK GEMSTART.S reassembled with STACK=32768
#                     (keep 32 KB stack+heap, return rest to GEMDOS
#                     so Malloc has room for the game's back-screen,
#                     letter template, SFX blocks, etc.)
#   $OBJS             all our compiled .o (main.o first)
#   vdibind.a         VDI C bindings          (DISK_2/LINKER/VDIBIND)
#   aesbind.a         AES C bindings          (DISK_2/LINKER/AESBIND)
#   osbind.o          _gemdos/_bios/_xbios    (DISK_2/LINKER/OSBIND.O)
#   gemlib.a          libc + __main + printf  (DISK_2/LINKER/GEMLIB)
#   libf              float formatters        (DISK_2/LINKER/LIBF)
#
# gemlib.a and libf are passed twice: lo68 scans archives once and
# __main pulls in code with forward refs to _etoa/_ftoa/etc.

set -euo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
ALCYON_BIN=${ALCYON_BIN:-$HOME/hatari-c/bin}
ATARI_DK=${ATARI_DK:-$HOME/Hatari_C/Compiler/ATARI_DK/DISK_2/LINKER}
DK_TOOLS=$CSRC/tools/dk
OUT=$CSRC/build/alcyon

cd "$OUT"

# 1. Reassemble GEMSTART.S (STACK=32768 baked in, long labels truncated
#    to <=8 chars for lo68 compatibility).  Rebuild only if missing or
#    the source is newer.
if [ ! -f gemstart.o ] || [ "$DK_TOOLS/gemstart.s" -nt gemstart.o ]; then
    cp -f "$DK_TOOLS/gemstart.s" gemstart.s
    "$ALCYON_BIN/as68" -l -u gemstart.s > /dev/null 2>&1 || {
        echo "FAILED: gemstart assembly"
        exit 1
    }
fi

# 1b. Assemble mq_tick.s -- the MFP Timer-A ISR, byte-faithful port of
#     Ghidra 0x1219a.  Assembly required: uses privileged
#     `move sr,dn` instructions (raise IPL to 7 on entry, lower to 5
#     during sub-calls) that Alcyon C 4.14 can't emit, and ends with
#     `rte`.  Installed directly by Xbtimer -- no C wrapper needed.
if [ ! -f mq_tick.o ] || [ "$DK_TOOLS/mq_tick.s" -nt mq_tick.o ]; then
    rm -f mq_hlpr.o mq_hlpr.s          # stale from earlier port scheme
    cp -f "$DK_TOOLS/mq_tick.s" mq_tick.s
    "$ALCYON_BIN/as68" -l -u mq_tick.s > /dev/null 2>&1 || {
        echo "FAILED: mq_tick assembly"
        exit 1
    }
fi

# 2. Copy DK libraries locally.
cp -f "$ATARI_DK/OSBIND.O"   osbind.o
cp -f "$ATARI_DK/AESBIND"    aesbind.a
cp -f "$ATARI_DK/VDIBIND"    vdibind.a
cp -f "$ATARI_DK/GEMLIB"     gemlib.a
cp -f "$ATARI_DK/LIBF"       libf

# 3. Build .o list (main.o and gemstart.o handled separately).
OBJS=""
for o in $(find . -maxdepth 1 -name "*.o" \
    ! -name "gemstart.o" ! -name "main.o" ! -name "osbind.o" \
    ! -name "crt0.o" ! -name "nofloat.o" | sort); do
    OBJS="$OBJS $(basename $o)"
done

# 4. Link.  -r emits relocation bits.  -s strips symbol table.
rm -f lcp.68k LCP.PRG
"$ALCYON_BIN/lo68" -r -s -o lcp.68k \
    gemstart.o main.o $OBJS \
    vdibind.a aesbind.a osbind.o gemlib.a libf gemlib.a libf 2>&1 | tail -20 || true

if [ ! -f lcp.68k ]; then
    echo "FAILED: lo68 didn't produce lcp.68k"
    exit 1
fi

# 5. relmod: .68k -> GEMDOS PRG.
"$ALCYON_BIN/relmod" lcp.68k LCP.PRG 2>&1 | tail -3
if [ ! -f LCP.PRG ]; then
    echo "FAILED: relmod didn't produce LCP.PRG"
    exit 1
fi

echo "SUCCESS: $OUT/LCP.PRG ($(stat -f%z LCP.PRG) bytes)"
