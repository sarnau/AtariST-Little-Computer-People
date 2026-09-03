/*
 * parts/a_wakum.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * ahouse functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_wakum()
{
        /* STX has no local: the tick count is used in place. */

        g_actif = YES;
        alarm_p = YES;
        gameTick(rndRng(40, 100));
        if (lcp.is_sleeping == YES)
                a_gioob();

        g_actif = YES; a_wakfa();
        g_actif = YES; a_takes();
        g_actif = YES; a_brust();
        g_actif = YES; a_opcbc(0);
        g_actif = YES; a_eatm();
        g_actif = NO;
}
