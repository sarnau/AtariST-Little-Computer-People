/*
 * parts/a_wakfa.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * asimple functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_wakfa()
{
        /* STX tests the walk call inline -- no local, so its frame
           is 2 bytes smaller. */

        hs_posXY(POS_MID_BEDROOM_WALK,
                              &g_wtx, &g_wty);
        if (lcp_wkD() == 0) {
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                alarm_p = NO;
        }
}
