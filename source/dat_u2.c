/*
 * dat_u2.c -- the initialized globals that belong to the stx_u2
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
 * Not compiled standalone -- included by stx_u2.
 */

/*
 * dat_u2.c -- the initialized globals that belong to the stx_u2
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
 * Not compiled standalone -- included by stx_u2.
 */

/*
 * dat_u2.c -- the initialized globals that belong to the stx_u2
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
 * Not compiled standalone -- included by stx_u2.
 */



BOOL16  g_rbact          = NO;

/* Three-letter abbreviations, not full names: the reference spends 48
   bytes here (twelve 4-byte strings, 0x1ef2..0x1f21), where the full
   names would take 86.  So the calendar and the letter date line read
   "Sep 4, 1985". */
char *  mo_names[12] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
};

BOOL16          moff_f = 1;

/* Openable-object frame-id slots, exactly in ROM data order
   (0x11758-0x1177e): study/front/cabinet/medicine/toilet doors,
   stove-off, the 3-slot stove-on table (g_obisa), then the fridge.
   od_draw sites read these slots, never enum constants. */
/* Six slots, and not the ids the port guessed: LCP_STX has
   { 43, 44, 45, 30, 31, 32 } here and a 12-byte gap. */
short   g_obisa[6]    = { 43, 44, 45, 30, 31, 32 };

/* TV pattern animation (Ghidra tv_pattern_N_x_coords / _y_coords).
   Four vertical scanlines drawn inside the TV screen -- each is a
   constant-X, descending-Y run of 8 points.  Colours picked from
   tv_pattern_color_indices (10, 5, 7, 13 in the main palette). */
short   g_tp0xc[8] = { 293, 293, 293, 293, 293, 293, 293, 293 };

short   g_tp0yc[8] = { 106, 105, 104, 103, 102, 101, 100,  99 };

short   g_tp1xc[8] = { 297, 297, 297, 297, 297, 297, 297, 297 };

short   g_tp1yc[8] = { 106, 105, 104, 103, 102, 101, 100,  99 };

short   g_tp2xc[8] = { 301, 301, 301, 301, 301, 301, 301, 301 };

short   g_tp2yc[8] = { 106, 105, 104, 103, 102, 101, 100,  99 };

short   g_tp3xc[8] = { 305, 305, 305, 305, 305, 305, 305, 305 };

short   g_tp3yc[8] = { 106, 105, 104, 103, 102, 101, 100,  99 };

short   g_tpcoi[4] = { 10, 5, 7, 13 };

short days_pmo[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* sprite_mfdb_image / sprite_mfdb_mask are Ghidra's names for the
   per-slot 8-way sprite MFDBs.  Our port already had them under the
   older names g_semfi / g_semfm (defined later in this file with
   { { 0 } } initializers) and referenced from sprender.c's sp_draw.
   sp_imfs writes through those existing arrays -- see sprites.c. */

/* Ghidra clock_minute @ 0x2B562 = 5, clock_hour @ 0x2B564 = 6.
   These are the "last-drawn" hand positions.  t_min/t_hour
   start at 0 (BSS), so the first cl_redrH call sees a mismatch
   and paints the initial 0:00 hands over the pre-drawn 5:06 default. */
short   g_cmmin                         = 5;

short   g_chhou                         = 6;

/* g_cmmip (Ghidra clock_minute_position @ 0x2B566, 15 shorts):
   circle-position table for the minute hand.  Indexed by the current
   minute/5 mod 12 giving one of 12 positions on a small circle around
   the clock centre; three padding entries at the end.  Values dumped
   live from Ghidra. */
short   g_cmmip[15] = {
         0,   2,   3,   3,   3,   2,   0,  -2,
        -3,  -3,  -3,  -2,   0,   2,   3
};

/* g_chhop (Ghidra clock_hour_position @ 0x2B584, 15 shorts): same
   shape for the hour hand, smaller radius (2 vs 3 pixels). */
short   g_chhop[15] = {
         0,   1,   2,   2,   2,   1,   0,  -1,
        -2,  -2,  -2,  -1,   0,   1,   2
};

/* mf_scrp now defined below with the rest of the frame-timing
   MFDB descriptors. */

/* Ghidra letter_line_count @ 0x2b5a2 = -1 (short).  First frame of
   rp_anim (record-player needle sweep) skips the draw when g_ltlic
   is < 0, then decrements to -3, then wraps to 13.  Port had 0. */
short   g_ltlic                         = -1;

short   g_ltpac          = 0;

/* rec_ledt[8]: bit-mask toggles for the VU-meter LEDs, high bit
   FIRST -- LCP_STX's table runs 0x80 down to 0x01 and its data gap
   here is 16 bytes. */
unsigned short  rec_ledt[8] = {
        0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};
