/*
 * parts/tt_on.c -- shared body; LCP_STX links it in the 0xdece object
 * (0x13bc8, immediately after a_toggt). Files under parts/ are never
 * compiled standalone.
 */

short

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
