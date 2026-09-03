#!/usr/bin/env bash
# alcyon_link.sh -- link with the 1985 Alcyon distribution that built
# LCP_STX (~/Hatari_C/Compiler/Alcyon/alcyon2, dated 1985-05-30).
#
# Composition:
#   gemstart.o        alcyon2 GEMSTART.O verbatim (LCP_STX's startup
#                     is this object, modulo relocation tails)
#   osbind.o          _gemdos/_bios/_xbios -- the trap bindings sit at
#                     text 0xfa, right behind gemstart
#   $OBJS             our compiled .o, in LCP_STX's object order
#   vdibind.a         VDI C bindings   (mostly shadowed by vdistx.o)
#   aesbind.a         AES C bindings
#   gemlib.a          libc + __main + printf
#
# gemlib.a is passed twice: link68 scans archives once and __main
# pulls in code with forward refs.  There is NO libf: LCP_STX's
# __pftoa/__petoa call _ftoa and _etoa at address ZERO, i.e. the 1985
# link left the two externals unresolved because the %f/%e printf
# conversions are unreachable in this program.  UNDEFINED reproduces
# that instead of failing the link.

set -euo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
ALCYON_BIN=${ALCYON_BIN:-$HOME/Hatari_C/hatari-c/bin}
DK_TOOLS=$CSRC
OUT=$CSRC/build/alcyon

cd "$OUT"

# 1b. Assemble mq_tick.s -- the MFP Timer-A ISR, byte-faithful port of
#     Ghidra 0x1219a.  Assembly required: uses privileged
#     `move sr,dn` instructions (raise IPL to 7 on entry, lower to 5
#     during sub-calls) that Alcyon C 4.14 can't emit, and ends with
#     `rte`.  Installed directly by Xbtimer -- no C wrapper needed.
# psg_asm.s: LCP_STX's hand-assembly psg_wr/psg_mix/mowrit.
if [ ! -f psg_asm.o ] || [ "$DK_TOOLS/psg_asm.s" -nt psg_asm.o ]; then
    cp -f "$DK_TOOLS/psg_asm.s" psg_asm.s
    "$ALCYON_BIN/as68" -l -u psg_asm.s > /dev/null 2>&1 || {
        echo "FAILED: psg_asm assembly"
        exit 1
    }
fi

# blkcp_a.s: LCP_STX's hand-assembly blkcp32 (0x17310).
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
# branch in the file carries its own explicit size.
if [ ! -f cp_asm.o ] || [ "$DK_TOOLS/cp_asm.s" -nt cp_asm.o ]; then
    cp -f "$DK_TOOLS/cp_asm.s" cp_asm.s
    "$ALCYON_BIN/as68" -l -u -n cp_asm.s > /dev/null 2>&1 || {
        echo "FAILED: cp_asm assembly"
        exit 1
    }
fi

if [ ! -f vdistx_a.o ] || [ "$DK_TOOLS/vdistx_a.s" -nt vdistx_a.o ]; then
    cp -f "$DK_TOOLS/vdistx_a.s" vdistx_a.s
    "$ALCYON_BIN/as68" -l -u vdistx_a.s > /dev/null 2>&1 || {
        echo "FAILED: vdistx_a assembly"
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

# 2. Copy the runtime libraries locally.  LCP_STX was built against
#    the 1985-05-30 alcyon2 distribution -- its startup IS that
#    GEMSTART.O, modulo relocation tails.
ALCYON2=${ALCYON2:-$HOME/Hatari_C/Compiler/Alcyon/alcyon2}
if [ ! -d "$ALCYON2" ]; then
    echo "FAILED: $ALCYON2 not found (LCP_STX's runtime libraries)"
    exit 1
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
    ! -name "gemstart.o" ! -name "main.o" ! -name "osbind.o" \
    ! -name "crt0.o" ! -name "nofloat.o" ! -name "psg_asm.o" ! -name "vdistx.o" \
    ! -name "vdistx_a.o" \
    ! -name "blkcp_a.o" ! -name "cp_asm.o" ! -name "mq_tick.o" | sort); do
    OBJS="$OBJS $(basename $o)"
done

# 4. Link.  -r emits relocation bits.  -s strips symbol table.
rm -f lcp.68k LCP.PRG
# LCP_STX object order.  The 1985 link laid the game objects down in
# this order, and every function's address depends on it:
#   0x0012a globals    0x0219a mq_tick   0x02272 psg_asm
#           (globals.o carries midi_seq.c -- see globals.c)
#   0x022c0 cp_asm     0x0400c stx_u1    0x073e8 games
#   0x0d9ea stx_u4     0x0de36 stx_u2    0x148fe stx_u3
#   0x17310 blkcp_a    0x1733a vdistx    then the library
# Everything not named here is an empty object (0 bytes of text) and
# is emitted first, where it costs nothing.
STXORDER="globals.o mq_tick.o psg_asm.o cp_asm.o stx_u1.o games.o \
          stx_u4.o stx_u2.o stx_u3.o blkcp_a.o vdistx.o vdistx_a.o"
REST=""
for o in $OBJS; do
    case " $STXORDER " in *" $o "*) ;; *) REST="$REST $o" ;; esac
done
OBJS="$REST $STXORDER"
# link68 (DRI CLI) with a response file.
# LCP_STX contains no LIBF code: its __pftoa/__petoa call _ftoa and
# _etoa at address 0, i.e. the 1985 link left them UNRESOLVED (the
# %f/%e printf conversions are never reached).  UNDEFINED reproduces
# that -- gemlib without libf, the two danglers resolved to 0 instead
# of failing the link.
TAIL="gemlib.a gemlib.a"
LINKOPT="[PRGFLAGS[0],UNDEFINED,COMMAND[lcp_link.cmd]]"
# The trap bindings sit at text 0xfa, right behind gemstart, so
# osbind.o goes there.  No separate workstation object: LCP_STX folds
# v_opnvwk/vro_cpyfm into the one binding module (vdistx.o) and has a
# single dispatcher, so there is no second parameter block.
LIST=$(echo "gemstart.o osbind.o main.o $OBJS vdibind.a aesbind.a $TAIL" | tr -s ' ' ',')
echo "lcp.68k=$LIST" > lcp_link.cmd
"$ALCYON_BIN/link68" "$LINKOPT" 2>&1 | tail -5 || true

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
