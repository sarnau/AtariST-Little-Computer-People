/*
 * dat_u1c.c -- the tail of stx_u1's initialized data.
 *
 * getKey's switch jump table lands in the data segment at LCP_STX
 * 0xba4..0xbe7 (17 longwords: sixteen scancode cases plus the default
 * at 0x6942), immediately after rv_val.  The three globals below
 * follow it, so they are declared after parts/getKey.c rather than
 * with the rest of dat_u1b.  Never compiled standalone.
 */

#include "types.h"
#include "enums.h"

short   g_clcos[16] = {
        0x060, 0x760, 0x606, 0x066,
        0x767, 0x007, 0x700, 0x030,
        0x767, 0x465, 0x314, 0x255,
        0x662, 0x406, 0x156, 0x514
};

/* g_clcop / g_clcos (Ghidra clothing_color_primary @ 0x2A2E4 and
   clothing_color_secondary @ 0x2A2C4): 16 pairs of primary +
   secondary 12-bit RGB shirt colours indexed by CLOTHING_COLOR_ID.
   Values dumped verbatim from Ghidra -- note the five duplicate blue
   primaries (0x006 for slots 0..4) which bias random clothing picks
   toward the same blue shirt. */
short   g_clcop[16] = {
        0x006, 0x006, 0x006, 0x006,
        0x006, 0x676, 0x676, 0x500,
        0x500, 0x735, 0x140, 0x641,
        0x623, 0x036, 0x242, 0x442
};

/* skin_pal[8] (Ghidra @ 0x2A304): SKIN_COLOR_ID (0..7),
   ST 12-bit RGB.  Values dumped verbatim from the data segment.
   Applied to palette slot 6 via lcp_upal and
   swapped in during the closet-change sequence in a_opcbc. */
short   skin_pal[8] = {
        0x512, 0x742, 0x567, 0x762,
        0x745, 0x145, 0x160, 0x565
};
