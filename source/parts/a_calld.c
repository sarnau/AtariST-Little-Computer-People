/*
 * parts/a_calld.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from asimple.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in asimple.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_calld()
{
        /* STX tests the walk call inline -- no local, so its frame
           is 2 bytes smaller. */
#ifdef FAITHFUL
        short   result;
#endif

        hs_posXY(POS_BTM_DOG_FOOD, &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif
        lcp_st              = STATE_STAND_SIDE_VIEW;
        lcp_face   = FACING_RIGHT;
        g_hatas = 8;
        lcp_hwt();
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(5);
        dg_petok = YES;
}
