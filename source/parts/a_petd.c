/*
 * parts/a_petd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * asimple functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_petd()
{
        short   ticks;

        g_actif = YES;
        if (dg_petok == NO)
                a_calld();
        g_actif = NO;

        ticks = rndRng(100, 200);
        if (introSeq != NO)
                ticks = 10;

        /* A pre-decrement while with the break inverted -- the
           subq's own flags, no tst. */
        while (--ticks != 0) {
                gameTick(0);
                if (g_trel[0] != ACTION_NONE)
                        break;
        }

        dg_petok = NO;
        lcp_st         = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
