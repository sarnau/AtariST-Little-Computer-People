/* Walk to fridge, then trampoline into putInFridge.

   Must sit directly before putInFridge, which it calls. */
void
goToFridge()
{
        /* The walk call is tested inline, with no local, as in the
           original. */

        posToXY(POS_BTM_FRIDGE, &walkXTarget, &walkYTarget);
        if (walkToTarget() == 0)
                putInFridge();
}
