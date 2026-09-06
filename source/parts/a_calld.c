/*
 * parts/a_calld.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * asimple functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_calld()
{
        /* STX tests the walk call inline -- no local, so its frame
           is 2 bytes smaller. */

        hs_posXY(POS_BTM_COUCH, &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        lcp_face   = FACING_RIGHT;
        g_hatas = 8;
        lcp_hwt();
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(5);
        pat_ok = YES;
}
