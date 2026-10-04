/*
 * Must sit directly before a_opecf, which it calls.
 *
 * Included by stx_u2.c; never compiled on its own.
 */

/* a_gesff: walk to fridge, then trampoline into a_opecf. */

void
a_gesff()
{
        /* The walk call is tested inline, with no local, as in the
           original. */

        hs_posXY(POS_BTM_FRIDGE,
                              &g_wtx, &g_wty);
        if (lcp_wkD() == 0)
                a_opecf();
}
