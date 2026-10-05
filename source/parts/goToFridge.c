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
                              &g_wtx, &g_wty);
        if (walkToTarget() == 0)
                putInFridge();
}
