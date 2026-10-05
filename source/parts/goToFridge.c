/*
 * Must sit directly before putInFridge, which it calls.
 *
 * Included by stx_u2.c; never compiled on its own.
 */

/* goToFridge: walk to fridge, then trampoline into putInFridge. */

void
goToFridge()
{
        /* The walk call is tested inline, with no local, as in the
           original. */

        posToXY(POS_BTM_FRIDGE,
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() == 0)
                putInFridge();
}
