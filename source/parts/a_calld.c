/*
 * parts/a_calld.c -- included by stx_u2.c; never compiled on its own.
 */

void
a_calld()
{
        /* The walk call is tested in place, with no local; adding
           one would change the compiled code. */

        hs_posXY(POS_BTM_COUCH, &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        lcp_face   = FACING_RIGHT;
        g_hatas = 8;
        lcp_hwt();
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(5);
        pat_ok = YES;
}
