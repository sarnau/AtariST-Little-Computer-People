/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* nightRoutine: the bedtime routine.  Chains five actions -- a shower (takeShower),
   changing in the bedroom closet (changeClothes(1), which also picks a new
   skin palette), a meal (eatFromCabinet), brushing teeth (brushTeeth) and
   getting into bed (getInOutOfBed) -- with noPreempt set before each so none
   of their walks can be preempted by a queued action. */
void
nightRoutine()
{
        noPreempt = YES; takeShower();
        noPreempt = YES; changeClothes(1);
        noPreempt = YES; eatFromCabinet();
        noPreempt = YES; brushTeeth();
        noPreempt = YES; getInOutOfBed();
        noPreempt = NO;
}
