/*
 * parts/a_eatm.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from afood.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in afood.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_eatm()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short   result;
#endif
#ifdef FAITHFUL
        short   counter;
        short   pick;
#else
        short   counter;
#endif

        hs_posXY(POS_BTM_KITCHEN_CABINET,
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

        lcp_st = STATE_BEND_DOWN;    gameTick(1);
        lcp_st = STATE_REACH_FORWARD;gameTick(2);
        lcp_st = STATE_STAND_FACING_SCREEN; gameTick(0);

        sp_ssco(SPRITE_COOKING_POT);
        hs_posXY(POS_BTM_STOVE,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();

        g_selaf[SPRITE_COOKING_POT] = SPRITE_HIDDEN;
        sp_upds();
        sp_ssco(SPRITE_COOKING_POT);
        g_lcyof = NO;
        g_sepex[g_seslm[SPRITE_COOKING_POT]] = 11;
        g_sepey[g_seslm[SPRITE_COOKING_POT]] = 172;

        lcp_face = FACING_LEFT;
        lcp_st            = STATE_BEND_AND_REACH;

        /* 30..50 tick cooking animation, rotating stove frames. */
#ifdef FAITHFUL
        counter = rndRng(30, 50);
        while (counter != 0) {
                pick = rndRng(0, 2);
                od_draw(g_obisa[pick], 6, 172);
                gameTick(1);
                counter = counter - 1;
        }
#else
        counter = rndRng(30, 50);
        while (counter-- != 0) {
                od_draw(g_obisa[rndRng(0, 2)], 6, 172);
                gameTick(1);
        }
#endif
        od_draw(od_stof, 6, 172);

        g_selaf[SPRITE_COOKING_POT] = SPRITE_HIDDEN;
        sp_upds();
        sp_ssco(SPRITE_COOKED_MEAL);

        /* Back to cabinet, then chain into a_kitcc to eat. */
        hs_posXY(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();
        g_selaf[SPRITE_COOKED_MEAL] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        gameTick(0);
        a_kitcc();
        g_actif = NO;
}
