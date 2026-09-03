/*
 * parts/a_wakfa.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from asimple.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in asimple.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_wakfa()
{
        /* STX tests the walk call inline -- no local, so its frame
           is 2 bytes smaller. */
#ifdef FAITHFUL
        short   result;
#endif

        hs_posXY(POS_MID_BEDROOM_WALK,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result == 0) {
#else
        if (lcp_wkD() == 0) {
#endif
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                alarm_p = NO;
        }
}
