/*
 * parts/a_wandi.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from aidle.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in aidle.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_wandi()
{
#ifndef FAITHFUL
        /* STX's frame is 2 bytes larger (link #-6 vs #-4): the
           original declared a local here that the body never uses. */
        short   unused;
#endif

        pst_arr[0]  = STATE_IDLE_SHRUG_START;
        pst_arr[1]  = STATE_IDLE_SHRUG_HOLD;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();

        lcp_st = pst_arr[0]; gameTick(2);
        lcp_st = pst_arr[1]; gameTick(5);
        lcp_st = pst_arr[0]; gameTick(2);
        lcp_st = STATE_STAND_SIDE_VIEW; gameTick(0);
}
