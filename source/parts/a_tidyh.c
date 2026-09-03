/*
 * parts/a_tidyh.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * aleisure functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_tidyh()
{
        /* STX tests the walk call inline -- no local, smaller frame. */

        hs_posXY(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_watat();

        /* STX has no local at all here: both call results are used
           in place, which is why its frame is 2 bytes smaller. */
        if (lcp.initiative_threshold < rndRng(0, 100) ||
            introSeq != NO)
                a_opcfc();
}
