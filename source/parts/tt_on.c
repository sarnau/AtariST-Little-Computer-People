/*
 * Included by stx_u2.c; never compiled on its own.
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
