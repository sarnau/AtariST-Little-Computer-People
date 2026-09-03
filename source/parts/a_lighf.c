/*
 * parts/a_lighf.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from aleisure.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in aleisure.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_lighf()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short   result;
#endif
        short   i;

        if (fire_act != NO)
                return;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_opcfd(0);
        g_actif = YES;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        g_wtx = g_wtx - 10;
#else
        g_wtx -= 10;
#endif
        lcp_wkD();

        /* Sit-dog sprite waits at the porch. */
        g_selaf[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hideLcp();
        gameTick(40);
        showLcp();

        sp_ssco(SPRITE_FIREWOOD);
        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        g_wtx = g_wtx - 10;
#else
        g_wtx -= 10;
#endif
        lcp_wkD();

        g_selaf[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        sp_upds();

#ifdef FAITHFUL
        result = rndRng(0, 100);
        if (lcp.initiative_threshold < result)
#else
        if (lcp.initiative_threshold < rndRng(0, 100))
#endif
                a_opcfd(1);

        hs_posXY(POS_BTM_FIREPLACE_LOGS,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        g_selaf[SPRITE_FIREWOOD] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_hwt();

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_BEND_DOWN;      gameTick(1);
        lcp_st = STATE_REACH_FORWARD;  gameTick(1);
        lcp_st = STATE_STOKE_FIREPLACE;gameTick(1);

        /* Random-direction shrug for 10 ticks (feeding kindling). */
#ifdef FAITHFUL
        for (i = 0; i < 10; i = i + 1) {
#else
        for (i = 0; i < 10; i++) {
#endif
                lcp_face = rndRng(0, 1);
                gameTick(0);
        }

        fire_act        = YES;
        fire_dur = rndRng(2500, 5000);

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
        g_actif = NO;
}
