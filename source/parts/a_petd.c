/*
 * parts/a_petd.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

void
a_petd()
{
        short   ticks;

        g_actif = YES;
        if (pat_ok == NO)
                a_calld();
        g_actif = NO;

        ticks = rndRng(100, 200);
        if (introSeq != NO)
                ticks = 10;

        /* Pre-decrement loop condition with the break inside: this
           shape is what the original compiles from; keep it. */
        while (--ticks != 0) {
                gameTick(0);
                if (g_trel[0] != ACTION_NONE)
                        break;
        }

        pat_ok = NO;
        lcp_st         = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
