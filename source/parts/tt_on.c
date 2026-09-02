/*
 * parts/tt_on.c -- shared body; LCP_ORG links it in render.c,
 * LCP_STX in the 0xdece object (0x13bc8, immediately after a_toggt).  Files under parts/
 * are never compiled standalone.
 */

short
#ifdef FAITHFUL
tt_on()
{
        short   result;

        if (lcp_tv != NO)
                return 0;

        hs_posXY(POS_TOP_LIVING_ROOM,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return -1;

        gameTick(2);
        li_lool();
        lcp_tv = YES;
        p_sftvc();
        return 0;
}
#else   /* STX: link #-4 -- no locals, the first early-out returns
           no value at all, and hs_posXY is followed by a no-op
           step on g_wtx. */

tt_on()
{
        if (lcp_tv != NO)
                return;

        hs_posXY(POS_TOP_LIVING_ROOM,
                              &g_wtx, &g_wty);
        g_wtx += 0;
        if (lcp_wkD() != 0)
                return -1;

        gameTick(2);
        li_lool();
        lcp_tv = YES;
        p_sftvc();
        return 0;
}
#endif
