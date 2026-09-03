/*
 * parts/a_pacen.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from aidle.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in aidle.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_pacen()
{
        short   i;

        pst_arr[0]  = STATE_PACE_SHIFT_LEFT;
        pst_arr[1]  = STATE_PACE_SHIFT_RIGHT;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();

        /* LCP_ORG's source uses the register form; the STX revision
           writes i++ (addq straight to the frame slot). */
#ifdef FAITHFUL
        for (i = 0; i < 15; i = i + 1) {
#else
        for (i = 0; i < 15; i++) {
#endif
                lcp_st = pst_arr[i & 1];
                gameTick(1);
        }
        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
