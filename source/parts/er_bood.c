/*
 * parts/er_bood.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * delivery functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
er_bood()
{
        g_actif = YES;
        wkFrDr();
        /* STX writes the pick-up sequence out in each handler --
           there is no dv_pick helper in that revision. */
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_opcfd(0);

        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_REACH_FORWARD;
        gameTick(2);
        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);

        if (lcp.initiative_threshold < rndRng(0, 100))
                a_opcfd(1);

        sp_ssco(SPRITE_BOOK);
        hs_posXY(POS_MID_BATHROOM_ENTRANCE,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_selaf[SPRITE_BOOK] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_face     = FACING_RIGHT;
        lcp_st                = STATE_STAND_FACING_SCREEN;
        g_hatas   = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        lcp_st = STATE_REACH_INTO_CABINET;
        gameTick(3);
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(2);
        g_actif = NO;
}
