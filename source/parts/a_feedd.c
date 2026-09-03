/*
 * parts/a_feedd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * afood functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_feedd(value)
short   value;
{
        /* STX tests the call in place -- no local. */

        if (value == 0) {
                hs_posXY(POS_BTM_FRIDGE,
                                      &g_wtx, &g_wty);
                if (lcp_wkD() != 0)
                        return;

                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();

                lcp_face = FACING_LEFT;
                lcp_st            = STATE_REACH_INTO_CABINET;
                od_draw(od_fdcl, 24, 153);
                gameTick(1);
                od_draw(od_fdo1, 24, 153);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(1);
                od_draw(od_fdo2, 24, 153);
                gameTick(1);

                lcp_face = FACING_RIGHT;
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(2);

                lcp_face = FACING_LEFT;
                lcp_st = STATE_REACH_INTO_CABINET;
                gameTick(3);

                lcp_face = FACING_RIGHT;
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(2);

                od_draw(od_fdo1, 24, 153);
                gameTick(1);
                od_draw(od_fdcl, 24, 153);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(1);

                sp_ssco(SPRITE_FOOD_PACKAGE);
        }

        hs_posXY(POS_BTM_DOG_BOWL,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_hwt();

        lcp_st = STATE_BEND_DOWN;    gameTick(1);
        lcp_st = STATE_REACH_FORWARD;gameTick(2);
        lcp_st = STATE_BEND_DOWN;    gameTick(1);

        dg_bwlch = 1;
        lcp_bwlS  = BOWL_FULL;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);

        sp_ssco(SPRITE_FOOD_PACKAGE);
        hs_posXY(POS_BTM_FRIDGE,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();

        g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        a_opecf();
        g_actif = NO;
}
