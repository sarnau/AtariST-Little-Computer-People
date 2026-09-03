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
 * SECOND of three parts.  stx_u1's data is NOT all-globals-then-code:
 * LCP_STX puts execEv's and doAct's switch jump tables at data
 * 0xa20..0xaf3, between pex_name and g_atact, and getKey's at
 * 0xba4..0xbe7 right after rv_val.  So these six globals are declared
 * between actions.c and parts/getKey.c in the unit, and dat_u1c's
 * three come after getKey.
 */

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
 * SECOND of three parts.  stx_u1's data is NOT all-globals-then-code:
 * LCP_STX puts execEv's and doAct's switch jump tables at data
 * 0xa20..0xaf3, between pex_name and g_atact, and getKey's at
 * 0xba4..0xbe7 right after rv_val.  So these six globals are declared
 * between actions.c and parts/getKey.c in the unit, and dat_u1c's
 * three come after getKey.
 */

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
