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
ALCYON_BIN=${ALCYON_BIN:-$HOME/Hatari_C/hatari-c/bin}
ATARI_DK=${ATARI_DK:-$HOME/Hatari_C/Compiler/ATARI_DK/DISK_2/LINKER}
DK_TOOLS=$CSRC
OUT=$CSRC/build/alcyon

cd "$OUT"

# 1. Reassemble GEMSTART.S (STACK=32768 baked in, long labels truncated
#    to <=8 chars for lo68 compatibility).  Rebuild only if missing or
#    the source is newer.
#    Assembled to gemstart_dk.o; the active gemstart.o is chosen in
#    step 2 (the default build uses alcyon2's GEMSTART.O verbatim).
if [ ! -f gemstart_dk.o ] || [ "$DK_TOOLS/gemstart.s" -nt gemstart_dk.o ]; then
    cp -f "$DK_TOOLS/gemstart.s" gemstart.s
    "$ALCYON_BIN/as68" -l -u gemstart.s > /dev/null 2>&1 || {
        echo "FAILED: gemstart assembly"
        exit 1
    }
    mv -f gemstart.o gemstart_dk.o
fi

# 1b. Assemble mq_tick.s -- the MFP Timer-A ISR, byte-faithful port of
#     Ghidra 0x1219a.  Assembly required: uses privileged
#     `move sr,dn` instructions (raise IPL to 7 on entry, lower to 5
#     during sub-calls) that Alcyon C 4.14 can't emit, and ends with
#     `rte`.  Installed directly by Xbtimer -- no C wrapper needed.
# vdiown_a.s: the STX configuration's separate vdi_go object (see the
# file header).  Assembled always; only linked when not FAITHFUL.
# psg_asm.s: LCP_STX's hand-assembly psg_wr/psg_mix/mowrit.  Linked
# only for the default build; FAITHFUL keeps psg_io.c's C versions.
if [ ! -f psg_asm.o ] || [ "$DK_TOOLS/psg_asm.s" -nt psg_asm.o ]; then
    cp -f "$DK_TOOLS/psg_asm.s" psg_asm.s
    "$ALCYON_BIN/as68" -l -u psg_asm.s > /dev/null 2>&1 || {
        echo "FAILED: psg_asm assembly"
        exit 1
    }
fi

# blkcp_a.s: LCP_STX's hand-assembly blkcp32 (0x17310).  Linked only
# for the default build; FAITHFUL keeps gfx_prim.c's C version.
if [ ! -f blkcp_a.o ] || [ "$DK_TOOLS/blkcp_a.s" -nt blkcp_a.o ]; then
    cp -f "$DK_TOOLS/blkcp_a.s" blkcp_a.s
    "$ALCYON_BIN/as68" -l -u blkcp_a.s > /dev/null 2>&1 || {
        echo "FAILED: blkcp_a assembly"
        exit 1
    }
fi

# cp_asm.s: LCP_STX's hand-assembly copy protection, cp_main
# (0x22c0-0x400b).  Assembled with -n (no branch optimization) because
# the original picks bsr.w in places where bsr.s would fit -- every
# branch in the file carries its own explicit size.  Linked only for
# the default build; FAITHFUL keeps stubs.c's 10-byte crack stub.
if [ ! -f cp_asm.o ] || [ "$DK_TOOLS/cp_asm.s" -nt cp_asm.o ]; then
    cp -f "$DK_TOOLS/cp_asm.s" cp_asm.s
    "$ALCYON_BIN/as68" -l -u -n cp_asm.s > /dev/null 2>&1 || {
        echo "FAILED: cp_asm assembly"
        exit 1
    }
fi

if [ ! -f vdiown_a.o ] || [ "$DK_TOOLS/vdiown_a.s" -nt vdiown_a.o ]; then
    cp -f "$DK_TOOLS/vdiown_a.s" vdiown_a.s
    "$ALCYON_BIN/as68" -l -u vdiown_a.s > /dev/null 2>&1 || {
        echo "FAILED: vdiown_a assembly"
        exit 1
    }
fi

if [ ! -f vdilib_a.o ] || [ "$DK_TOOLS/vdilib_a.s" -nt vdilib_a.o ]; then
    cp -f "$DK_TOOLS/vdilib_a.s" vdilib_a.s
    "$ALCYON_BIN/as68" -l -u vdilib_a.s > /dev/null 2>&1 || {
        echo "FAILED: vdilib_a assembly"
        exit 1
    }
fi

if [ ! -f mq_tick.o ] || [ "$DK_TOOLS/mq_tick.s" -nt mq_tick.o ]; then
    rm -f mq_hlpr.o mq_hlpr.s          # stale from earlier port scheme
    cp -f "$DK_TOOLS/mq_tick.s" mq_tick.s
    "$ALCYON_BIN/as68" -l -u mq_tick.s > /dev/null 2>&1 || {
        echo "FAILED: mq_tick assembly"
        exit 1
    }
fi

# 2. Copy the runtime libraries locally.  FAITHFUL links the Atari
#    Developer Kit set (LCP_ORG's); the default build links the older
#    alcyon2 distribution's, which is what LCP_STX was built against
#    (see "Toolchain differs from LCP_ORG's" in CLAUDE.md).
ALCYON2=${ALCYON2:-$HOME/Hatari_C/Compiler/Alcyon/alcyon2}
if [ "${FAITHFUL:-0}" = "1" ] || [ ! -d "$ALCYON2" ]; then
    cp -f "$ATARI_DK/OSBIND.O"   osbind.o
    cp -f "$ATARI_DK/AESBIND"    aesbind.a
    cp -f "$ATARI_DK/VDIBIND"    vdibind.a
    cp -f "$ATARI_DK/GEMLIB"     gemlib.a
    cp -f "$ATARI_DK/LIBF"       libf
    cp -f gemstart_dk.o          gemstart.o
else
    cp -f "$ALCYON2/OSBIND.O"    osbind.o
    cp -f "$ALCYON2/AESBIND"     aesbind.a
    cp -f "$ALCYON2/VDIBIND"     vdibind.a
    cp -f "$ALCYON2/GEMLIB"      gemlib.a
    cp -f "$ALCYON2/LIBF"        libf
    cp -f "$ALCYON2/GEMSTART.O"  gemstart.o
fi

# 3. Build .o list (main.o and gemstart.o handled separately).
OBJS=""
for o in $(find . -maxdepth 1 -name "*.o" \
    ! -name "gemstart.o" ! -name "main.o" ! -name "osbind.o" ! -name "gemstart_dk.o" \
    ! -name "crt0.o" ! -name "nofloat.o" ! -name "vdilib.o" ! -name "vdilib_a.o" \
    ! -name "vdiown_a.o" ! -name "psg_asm.o" \
    ! -name "blkcp_a.o" ! -name "cp_asm.o" | sort); do
    OBJS="$OBJS $(basename $o)"
done

# 4. Link.  -r emits relocation bits.  -s strips symbol table.
rm -f lcp.68k LCP.PRG
# vdilib.o sits in library position, exactly where the ROM links its
# own workstation module (0xe754), ahead of vdibind.a which it shadows.
# FAITHFUL=1 drops mq_tick.o -- the ROM has no Timer-A ISR.
if [ "${FAITHFUL:-0}" = "1" ]; then
    OBJS=$(echo " $OBJS " | sed 's/ mq_tick.o / /')
else
    OBJS="$OBJS vdiown_a.o psg_asm.o blkcp_a.o cp_asm.o"  # STX: vdi_go + asm
fi
# link68 (DRI CLI) with a response file; same object order as lo68 had.
# LCP_STX contains no LIBF code: its __pftoa/__petoa call _ftoa and
# _etoa at address 0, i.e. the 1985 link left them UNRESOLVED (the
# %f/%e printf conversions are never reached).  The default build
# reproduces that -- gemlib without libf, linked with UNDEFINED so
# link68 resolves the two dangling externals to 0 instead of failing.
if [ "${FAITHFUL:-0}" = "1" ]; then
    TAIL="gemlib.a libf gemlib.a libf"
    LINKOPT="[PRGFLAGS[0],COMMAND[lcp_link.cmd]]"
else
    TAIL="gemlib.a gemlib.a"
    LINKOPT="[PRGFLAGS[0],UNDEFINED,COMMAND[lcp_link.cmd]]"
fi
LIST=$(echo "gemstart.o main.o $OBJS vdilib.o vdilib_a.o vdibind.a aesbind.a osbind.o $TAIL" | tr -s ' ' ',')
echo "lcp.68k=$LIST" > lcp_link.cmd
"$ALCYON_BIN/link68" "$LINKOPT" 2>&1 | tail -5 || true

# 4b. FAITHFUL: unstripped side link -- bss_remap.py resolves the
#     rom_bss_layout.tsv symbol keys against its symbol table.
if [ "${FAITHFUL:-0}" = "1" ]; then
    rm -f lcp_sym.68k
    echo "lcp_sym.68k=$LIST" > lcp_sym.cmd
    "$ALCYON_BIN/link68" "[PRGFLAGS[0],SYMBOLS,COMMAND[lcp_sym.cmd]]" 2>&1 | tail -3 || true
    [ -f lcp_sym.68k ] || { echo "FAILED: symbol side link"; exit 1; }
fi

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

# 6. FAITHFUL: re-lay BSS to the original linker's allocation.  lo68
#    allocates .comm blocks hash-grouped; the 1985 linker used an
#    order no surviving tool reproduces.  bss_remap.py applies the
#    checked-in layout spec tools/rom_bss_layout.tsv (symbol+offset
#    -> ROM address), resolved via the lcp_sym.68k side link -- the
#    original binary is not read.  See the header of
#    tools/bss_remap.py (and its --gen mode to regenerate the spec).
if [ "${FAITHFUL:-0}" = "1" ]; then
    python3 "$CSRC/tools/bss_remap.py" LCP.PRG || {
        echo "FAILED: bss_remap"
        exit 1
    }
fi

echo "SUCCESS: $OUT/LCP.PRG ($(stat -f%z LCP.PRG) bytes)"
