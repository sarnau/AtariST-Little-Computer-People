/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* a_gotbn: the bedtime routine.  Chains five actions -- a shower (a_takes),
   changing in the bedroom closet (a_opcbc(1), which also picks a new
   skin palette), a meal (a_kitcc), brushing teeth (a_brust) and
   getting into bed (a_gioob) -- with g_actif set before each so none
   of their walks can be preempted by a queued action. */
void
a_gotbn()
{
        g_actif = YES; a_takes();
        g_actif = YES; a_opcbc(1);
        g_actif = YES; a_kitcc();
        g_actif = YES; a_brust();
        g_actif = YES; a_gioob();
        g_actif = NO;
}
