/*
 * dat_u2.c -- the initialized globals that belong to the stx_u2
 * object, in the original's data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The position of
 * this file's #include and the order of the declarations below set
 * the data layout; do not reorder them.
 *
 * Not compiled standalone -- included by stx_u2.c.
 */

BOOL16  g_rbact          = NO;

/* Three-letter abbreviations, so the calendar and the letter date
   line read "Sep 4, 1985". */
char *  mo_names[12] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
};

BOOL16          moff_f = 1;   /* YES while the mouse is hidden; moff/mon only call graf_mouse on a change.  Starts YES, so the first moff is a no-op until mon has shown it */

/* Object frame ids; the od_draw call sites read these slots rather
   than enum constants. */
short   g_obisa[6]    = { 43, 44, 45, 30, 31, 32 };

/* TV pattern animation.
   Four vertical scanlines drawn inside the TV screen -- each is a
   constant-X, descending-Y run of 8 points.  Colours picked from
   g_tpcoi (10, 5, 7, 13 in the main palette). */
short   g_tp0xc[8] = { 293, 293, 293, 293, 293, 293, 293, 293 };

short   g_tp0yc[8] = {   /* bar 0 Y per step (one row up per step) */ 106, 105, 104, 103, 102, 101, 100,  99 };

short   g_tp1xc[8] = {   /* bar 1 X per step */ 297, 297, 297, 297, 297, 297, 297, 297 };

short   g_tp1yc[8] = {   /* bar 1 Y per step */ 106, 105, 104, 103, 102, 101, 100,  99 };

short   g_tp2xc[8] = {   /* bar 2 X per step */ 301, 301, 301, 301, 301, 301, 301, 301 };

short   g_tp2yc[8] = {   /* bar 2 Y per step */ 106, 105, 104, 103, 102, 101, 100,  99 };

short   g_tp3xc[8] = {   /* bar 3 X per step */ 305, 305, 305, 305, 305, 305, 305, 305 };

short   g_tp3yc[8] = {   /* bar 3 Y per step */ 106, 105, 104, 103, 102, 101, 100,  99 };

short   g_tpcoi[4] = {   /* colour index per bar (through vdi_colt) */ 10, 5, 7, 13 };

/* Days per month, January first; daysInMo replaces February's 28 by
   29 in leap years. */
short days_pmo[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* The "last-drawn" clock hand positions (minute 5, hour 6).  t_min/t_hour
   start at 0 (BSS), so the first cl_redrH call sees a mismatch
   and paints the initial 0:00 hands over the pre-drawn 5:06 default. */
short   g_cmmin                         = 5;

short   g_chhou                         = 6;   /* hour the clock's hour hand was last drawn for */

/* Circle-position table for the minute hand.  Indexed by the current
   minute/5 mod 12 giving one of 12 positions on a small circle around
   the clock centre; three padding entries at the end. */
short   g_cmmip[15] = {
         0,   2,   3,   3,   3,   2,   0,  -2,
        -3,  -3,  -3,  -2,   0,   2,   3
};

/* Same shape for the hour hand, smaller radius (2 vs 3 pixels). */
short   g_chhop[15] = {
         0,   1,   2,   2,   2,   1,   0,  -1,
        -2,  -2,  -2,  -1,   0,   1,   2
};

/* Starts at -1: the first frame of rp_anim (record-player needle
   sweep) skips the draw when g_ltlic is < 0, then decrements to -3,
   then wraps to 13. */
short   g_ltlic                         = -1;

short   g_ltpac          = 0;   /* record player VU LEDs currently lit, one bit per LED (rec_ledt); rp_anim toggles them */

/* Bit-mask toggles for the VU-meter LEDs, high bit first. */
unsigned short  rec_ledt[8] = {
        0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};
