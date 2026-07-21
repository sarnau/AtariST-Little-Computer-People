/* tables.c -- static ROM data tables (dumped from LCP.PRG via Ghidra). */

#include "types.h"
#include "enums.h"
#include "tables.h"

/* g_rpxs[48]: X half-pixel coordinate per HOUSE_POS.
   Table value gets left-shifted by 1 at the call site to yield the
   full-pixel X (see hs_posXY).
   addr: g_rpxs at 0x019eb2 */
short   g_rpxs[48] = {
        /* Floor 3 -- top       0..15 */
         22,  36,  49,  55,  60,  56,  73,  96,
        106, 118, 113, 110, 131,  47, 133, 146,
        /* Floor 2 -- middle   16..31 */
         16,  40,  27,  31,  45,  55,  84, 100,
        111, 100, 109, 124, 134, 135, 144,  67,
        /* Floor 1 -- bottom   32..47 */
          8,   8,  12,  19,  40,  25,  54,  49,
         67,  70, 106, 110, 123, 132, 147, 140
};

/* rev_tab[256]: 8-bit bit-reversal LUT used to mirror sprites
   horizontally.  In the ROM this is BSS at 0x4d00e, filled at
   startup by build_bit_revert_table (Ghidra 0x1680e) using the two
   bitmask tables below.  Port matches: uninitialised here, populated
   by initBRev() at boot before any sprite pipeline call.
   addr: revert_table @ 0x4d00e */
unsigned short  rev_tab[256];

/* bm_msb_lsb[8] / bm_lsb_msb[8]: source and destination bit-position
   masks used by build_bit_revert_table to build rev_tab.
   Ghidra names:
     bitmask_80_40_20_10_8_4_2_1 @ 0x2a260  (input bit selectors)
     bitmask_1_2_4_8_10_20_40_80 @ 0x2a270  (mirrored output bits)
   Both are 8 words each (move.w operands in the reversal loop). */
unsigned short  bm_msb_lsb[8] = {
        0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};
unsigned short  bm_lsb_msb[8] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

/* AI action tables: 16 ACTION_IDs each, picked by chk_timA() at the
   active/moderate/relaxed tier.
   Ghidra 0x2a1d0 / 0x2a1f0 / 0x2a210. */

short   g_atact[16] = {
        27, 36,  2,  7, 37, 19, 30, 23,
        24,  0,  2, 36, 19, 38,  2, 37
};

short   g_atmod[16] = {
        24,  8, 38, 39,  5, 26, 30, 39,
         1, 10, 16, 27,  0, 24,  8, 30
};

short   g_atrel[16] = {
         1, 42, 20,  5, 27, 39, 30, 19,
        24, 12, 19, 42, 38,  6,  1, 39
};

/* sch_tab[3][8] (Ghidra 0x2a230): (phase, activity_level) -> bucket.
   Indexed via *(sch_tab + hours_bucket*16 + activity_level*2).
   Row names shortened for Alcyon as68 8-char symbol truncation. */
static short    schr0[8] = { 0, 0, 2, 2, 1, 1, 0, 1 };
static short    schr1[8] = { 2, 1, 0, 1, 2, 0, 2, 0 };
static short    schr2[8] = { 1, 2, 1, 0, 0, 2, 1, 2 };
short *         sch_tab[3] = {
        schr0, schr1, schr2
};

/* g_rphs[49] (Ghidra 0x29F2A): Y offset from floor baseline per
   HOUSE_POS.  Entry 0 = 140 (ground-floor sentinel). */
short   g_rphs[49] = {
        140,   9,  14,   9,  10,  11,  14,  12,
         13,  12,  12,  12,   6,  15,  10,  14,
          3,   3,   3,   8,  15,  13,  13,  12,
         13,  14,  12,   8,  14,  13,  14,  13,
          5,   8,   3,  10,  13,  13,  14,  10,
         14,  14,  12,  13,   7,  14,  12,  13,
          2
};

/* bm32or[i] = 1<<i, bm32and[i] = ~(1<<i).  Populated at boot by initBM
   (Alcyon rejects 'UL' suffix on hex > 0x7FFFFFFF, so no data literals).
   addr: bm32or, bm32and */

long    bm32or[32];
long    bm32and[32];

void
initBM()
{
        short   i;
        long    v;

        v = 1L;
        for (i = 0; i < 32; i = i + 1) {
                bm32or[i]  = v;
                bm32and[i] = ~v;
                v = v << 1;
        }
}
