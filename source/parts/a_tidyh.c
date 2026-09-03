/*
 * parts/a_tidyh.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from aleisure.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in aleisure.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_tidyh()
{
        /* STX tests the walk call inline -- no local, smaller frame. */
#ifdef FAITHFUL
        short   result;
#endif

        hs_posXY(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_watat();

#ifdef FAITHFUL
        result = rndRng(0, 100);
        if (lcp.initiative_threshold < result ||
            introSeq != NO)
                a_opcfc();
#else
        /* STX has no local at all here: both call results are used
           in place, which is why its frame is 2 bytes smaller. */
        if (lcp.initiative_threshold < rndRng(0, 100) ||
            introSeq != NO)
                a_opcfc();
#endif
}
