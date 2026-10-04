/*
 * parts/a_wakum.c -- the morning routine after the alarm.
 * Included by stx_u2.c; never compiled on its own.
 */

void
a_wakum()
{
        /* No local: the tick count is passed straight through. */

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
