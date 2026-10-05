/*
 * dat_colors.c -- the tail of stx_u1's initialized data.
 *
 * getKey's switch jump table lands in the data segment immediately
 * after mirrorDstBit, and the three globals below follow it, so they are
 * declared after parts/getKey.c rather than with the rest of dat_aitables.
 * Their order here is the data layout and must not change.  Never
 * compiled standalone.
 */

#include "types.h"
#include "enums.h"

/* Secondary shirt colour (12-bit ST RGB) per CLOTHING_COLOR_ID;
   pickClothes loads it into palette slot 2 alongside shirtPrimary (below). */
short   shirtSecondary[16] = {
        0x060, 0x760, 0x606, 0x066,
        0x767, 0x007, 0x700, 0x030,
        0x767, 0x465, 0x314, 0x255,
        0x662, 0x406, 0x156, 0x514
};

/* shirtPrimary / shirtSecondary: 16 pairs of primary + secondary 12-bit RGB
   shirt colours indexed by CLOTHING_COLOR_ID.  Note the five duplicate blue
   primaries (0x006 for slots 0..4) which bias random clothing picks
   toward the same blue shirt. */
short   shirtPrimary[16] = {
        0x006, 0x006, 0x006, 0x006,
        0x006, 0x676, 0x676, 0x500,
        0x500, 0x735, 0x140, 0x641,
        0x623, 0x036, 0x242, 0x442
};

/* skinColors[8]: SKIN_COLOR_ID (0..7), ST 12-bit RGB.  Applied to palette slot 6 via setSkinColor and
   swapped in during the closet-change sequence in changeClothes. */
short   skinColors[8] = {
        0x512, 0x742, 0x567, 0x762,
        0x745, 0x145, 0x160, 0x565
};
