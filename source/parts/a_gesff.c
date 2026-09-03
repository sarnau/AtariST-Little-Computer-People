/*
 * parts/a_gesff.c -- shared body; LCP_STX places it at 0xebf8, between a_clocd (0xeb54) and
 * a_opecf (0xec22), which is why its call to a_opecf is a SHORT
 * bsr.
 */

/* a_gesff: walk to fridge, then trampoline into a_opecf.
   addr: a_gesff() */

void
a_gesff()
{
        /* STX tests the walk call inline -- no local, so its frame
           is 2 bytes smaller. */

        hs_posXY(POS_BTM_FRIDGE,
                              &g_wtx, &g_wty);
        if (lcp_wkD() == 0)
                a_opecf();
}
