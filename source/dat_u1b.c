/*
 * dat_u1b.c -- the initialized globals that belong to the stx_u1
 * OBJECT, in LCP_STX data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The object that
 * owns a stretch of anonymous data is not a guess: a switch table's
 * relocation points into its own function, and a string is emitted in
 * the object that references it.  See CLAUDE.md, "DATA and BSS
 * layout".
 *
 * Not compiled standalone -- included by stx_u1.
 *
 * SECOND half.  stx_u1's data is NOT all-globals-then-code: LCP_STX
 * puts execEv's and doAct's switch jump tables at data 0xa20..0xaf3,
 * between pex_name and g_atact.  So these nine globals are declared
 * AFTER ai.c and actions.c in the unit, not with the rest at the top.
 */

/* AI action tables: 16 ACTION_IDs each, picked by chk_timA() at the
   active/moderate/relaxed tier.
   Ghidra 0x2a1d0 / 0x2a1f0 / 0x2a210. */

short   g_atact[16] = {
        27, 36,  2,  7, 37, 19, 30, 23,
        24,  0,  2, 36, 19, 38,  2, 37
};

/* skin_pal[8] (Ghidra @ 0x2A304): SKIN_COLOR_ID (0..7),
   ST 12-bit RGB.  Values dumped verbatim from the data segment.
   Applied to palette slot 6 via lcp_upal and
   swapped in during the closet-change sequence in a_opcbc. */
short   skin_pal[8] = {
        0x512, 0x742, 0x567, 0x762,
        0x745, 0x145, 0x160, 0x565
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
