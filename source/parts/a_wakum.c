/*
 * parts/a_wakum.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from ahouse.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in ahouse.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_wakum()
{
        /* STX has no local: the tick count is used in place. */
#ifdef FAITHFUL
        short   counter;
#endif

        g_actif = YES;
        alarm_p = YES;
#ifdef FAITHFUL
        counter = rndRng(40, 100);
        gameTick(counter);
#else
        gameTick(rndRng(40, 100));
#endif
        if (lcp.is_sleeping == YES)
                a_gioob();

        g_actif = YES; a_wakfa();
        g_actif = YES; a_takes();
        g_actif = YES; a_brust();
        g_actif = YES; a_opcbc(0);
        g_actif = YES; a_eatm();
        g_actif = NO;
}
