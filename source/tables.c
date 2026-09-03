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

/* rev_tab[256]: 8-bit bit-reversal LUT used to mirror sprites.
   LCP_STX does NOT ship this as data -- initBRev builds it at boot
   from the two 8-entry bit tables below (mask MSB-first, value
   LSB-first), so the array lives in BSS. */
short           rev_tab[256];

/* g_rphs[48]: Y offset from floor baseline per HOUSE_POS.  There is
   no leading 140 "ground-floor sentinel" -- LCP_STX's table starts at
   9 and its data gap here is 96 bytes = 48 shorts. */
short   g_rphs[48] = {
          9,  14,   9,  10,  11,  14,  12,  13,
         12,  12,  12,   6,  15,  10,  14,   3,
          3,   3,   8,  15,  13,  13,  12,  13,
         14,  12,   8,  14,  13,  14,  13,   5,
          8,   3,  10,  13,  13,  14,  10,  14,
         14,  12,  13,   7,  14,  12,  13,   2
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
/* LCP_STX indexes a flat array here, not a row-pointer table: its
   chk_timA adds an immediate base and the table sits directly after
   g_atrel in data (106960/106992/107024/107056, 32 bytes apart). */
short           sch_tab[3][8] = {
        { 0, 0, 2, 2, 1, 1, 0, 1 },
        { 2, 1, 0, 1, 2, 0, 2, 0 },
        { 1, 2, 1, 0, 0, 2, 1, 2 }
};

short           rv_msk[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
short           rv_val[8] = {   1,  2,  4,  8, 16, 32, 64, 128 };

/* bm32or[i] = 1<<i, bm32and[i] = ~(1<<i).  LCP_STX has no builder for
   these -- there is not a single `not.l` in its whole text -- because
   it ships them as initialized DATA instead.
   addr: bm32or, bm32and */

/* LCP_STX ships both tables as initialized DATA rather than
   building them at run time. */
long    bm32or[32] = {
        0x00000001L,
        0x00000002L,
        0x00000004L,
        0x00000008L,
        0x00000010L,
        0x00000020L,
        0x00000040L,
        0x00000080L,
        0x00000100L,
        0x00000200L,
        0x00000400L,
        0x00000800L,
        0x00001000L,
        0x00002000L,
        0x00004000L,
        0x00008000L,
        0x00010000L,
        0x00020000L,
        0x00040000L,
        0x00080000L,
        0x00100000L,
        0x00200000L,
        0x00400000L,
        0x00800000L,
        0x01000000L,
        0x02000000L,
        0x04000000L,
        0x08000000L,
        0x10000000L,
        0x20000000L,
        0x40000000L,
        0x80000000L
};

long    bm32and[32] = {
        0xfffffffeL,
        0xfffffffdL,
        0xfffffffbL,
        0xfffffff7L,
        0xffffffefL,
        0xffffffdfL,
        0xffffffbfL,
        0xffffff7fL,
        0xfffffeffL,
        0xfffffdffL,
        0xfffffbffL,
        0xfffff7ffL,
        0xffffefffL,
        0xffffdfffL,
        0xffffbfffL,
        0xffff7fffL,
        0xfffeffffL,
        0xfffdffffL,
        0xfffbffffL,
        0xfff7ffffL,
        0xffefffffL,
        0xffdfffffL,
        0xffbfffffL,
        0xff7fffffL,
        0xfeffffffL,
        0xfdffffffL,
        0xfbffffffL,
        0xf7ffffffL,
        0xefffffffL,
        0xdfffffffL,
        0xbfffffffL,
        0x7fffffffL
};
