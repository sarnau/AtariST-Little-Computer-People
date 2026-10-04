/*
 * parts/a_petd.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* a_petd: wait to be patted.  Unless pat_ok is already set, the
   resident first goes to the couch and crouches (a_calld).  He then
   waits 100..200 ticks (10 during the intro), or until a new action is
   queued, for the player's Ctrl-P; afterwards pat_ok is cleared and he
   stands up. */
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
