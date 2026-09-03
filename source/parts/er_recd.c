/*
 * parts/er_recd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from delivery.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in delivery.c.
 * Files under parts/ are never compiled standalone.
 */

void
er_recd()
{
#ifndef FAITHFUL
        short   unused;         /* STX: link #-6, the slot is never written */
#endif

        g_actif = YES;
        wkFrDr();
#ifdef FAITHFUL
        dv_pick();
#else
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
#endif

        sp_ssco(SPRITE_VINYL_CARRY);
        hs_posXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        g_selaf[SPRITE_VINYL_CARRY] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_hwt();

        lcp_st = STATE_BEND_DOWN;    gameTick(1);
        lcp_st = STATE_REACH_FORWARD; gameTick(2);
        lcp_st = STATE_BEND_DOWN;    gameTick(1);
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);

#ifdef FAITHFUL
        lcp_food = lcp_food + 1;    /* 1985 typo, preserved */
#else
        lcp_food++;                 /* 1985 typo, preserved */
#endif
        g_actif = NO;
}
