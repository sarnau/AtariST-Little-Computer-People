/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* paceNervously: pace nervously on the spot.  The resident turns side-on,
   waits for his head to settle, then alternates the shift-left and
   shift-right pacing poses for 15 ticks and stands still again. */
void
paceNervously()
{
        short   i;

        pst_arr[0]  = STATE_PACE_SHIFT_LEFT;
        pst_arr[1]  = STATE_PACE_SHIFT_RIGHT;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        waitHeadTurn();

        /* Written i++ on purpose: i = i + 1 compiles differently. */
        for (i = 0; i < 15; i++) {
                lcp_st = pst_arr[i & 1];
                gameTick(1);
        }
        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
