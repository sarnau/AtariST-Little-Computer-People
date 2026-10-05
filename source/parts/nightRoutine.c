/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* nightRoutine: the bedtime routine.  Chains five actions -- a shower (takeShower),
   changing in the bedroom closet (changeClothes(1), which also picks a new
   skin palette), a meal (eatFromCabinet), brushing teeth (brushTeeth) and
   getting into bed (getInOutOfBed) -- with g_actif set before each so none
   of their walks can be preempted by a queued action. */
void
nightRoutine()
{
        g_actif = YES; takeShower();
        g_actif = YES; changeClothes(1);
        g_actif = YES; eatFromCabinet();
        g_actif = YES; brushTeeth();
        g_actif = YES; getInOutOfBed();
        g_actif = NO;
}
