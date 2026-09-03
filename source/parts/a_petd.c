/*
 * parts/a_petd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from asimple.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in asimple.c.
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

        /* Same loop, two source shapes: LCP_ORG spells out the
           decrement-and-break, the STX revision uses a pre-decrement
           while with the break inverted. */
#ifdef FAITHFUL
        do {
                ticks = ticks - 1;
                if (ticks == 0)
                        break;
                gameTick(0);
        } while (g_trel[0] == ACTION_NONE);
#else
        while (--ticks != 0) {
                gameTick(0);
                if (g_trel[0] != ACTION_NONE)
                        break;
        }
#endif

        dg_petok = NO;
        lcp_st         = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
