/*
 * parts/er_food.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from delivery.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in delivery.c.
 * Files under parts/ are never compiled standalone.
 */

#ifdef FAITHFUL
void
er_food()
{
        unsigned short  food_count;
        short           roll;

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

#ifdef FAITHFUL
        if (g_dvdog == NO) {
                sp_ssco(SPRITE_FOOD_PACKAGE);
                hs_posXY(POS_BTM_KITCHEN_CABINET,
                                      &g_wtx, &g_wty);
                lcp_wkD();

                g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
                sp_upds();
                g_lcyof = NO;
                lcp_face     = FACING_RIGHT;
                lcp_st                = STATE_STAND_FACING_SCREEN;
                g_hatas   = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();

                a_opecc(0);

                /* Stock the cabinet: the 3-bit food count lives at
                   bits 9..11 of door_states_and_flags.  Bump it up to
                   4 packs, one visible reach-in per pack. */
                for (;;) {
                        food_count = ((lcp.door_states_and_flags >> 9) & 7)
                                     + 1;
                        if (food_count >= 5)
                                break;
                        lcp.door_states_and_flags =
                                (food_count * 0x200) |
                                (lcp.door_states_and_flags & ~DSF_FOOD_MASK);
                        lcp_st = STATE_REACH_INTO_CABINET;
                        gameTick(3);
                        sc_drfc();
                        lcp_st = STATE_STAND_FACING_SCREEN;
                        gameTick(1);
                }

                roll = rndRng(0, 100);
                if (lcp.initiative_threshold < roll)
                        a_opecc(1);
                g_actif = NO;
        } else {
                sp_ssco(SPRITE_FOOD_PACKAGE);
                if (lcp_bwlS == BOWL_EMPTY) {
                        a_feedd(1);
                } else {
                        a_gesff();
                        g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
                        sp_upds();
                        g_lcyof = NO;
                }
        }
#else   /* STX tests the other way and swaps the arms. */
        if (g_dvdog != NO) {
                sp_ssco(SPRITE_FOOD_PACKAGE);
                if (lcp_bwlS == BOWL_EMPTY) {
                        a_feedd(1);
                } else {
                        a_gesff();
                        g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
                        sp_upds();
                        g_lcyof = NO;
                }
        } else {
                sp_ssco(SPRITE_FOOD_PACKAGE);
                hs_posXY(POS_BTM_KITCHEN_CABINET,
                                      &g_wtx, &g_wty);
                lcp_wkD();

                g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
                sp_upds();
                g_lcyof = NO;
                lcp_face     = FACING_RIGHT;
                lcp_st                = STATE_STAND_FACING_SCREEN;
                g_hatas   = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();

                a_opecc(0);

                /* Stock the cabinet: the 3-bit food count lives at
                   bits 9..11 of door_states_and_flags.  Bump it up to
                   4 packs, one visible reach-in per pack. */
                for (;;) {
                        food_count = ((lcp.door_states_and_flags >> 9) & 7)
                                     + 1;
                        if (food_count >= 5)
                                break;
                        lcp.door_states_and_flags =
                                (food_count * 0x200) |
                                (lcp.door_states_and_flags & ~DSF_FOOD_MASK);
                        lcp_st = STATE_REACH_INTO_CABINET;
                        gameTick(3);
                        sc_drfc();
                        lcp_st = STATE_STAND_FACING_SCREEN;
                        gameTick(1);
                }

                roll = rndRng(0, 100);
                if (lcp.initiative_threshold < roll)
                        a_opecc(1);
                g_actif = NO;
        }
#endif
}
#else   /* STX: the pick-up sequence is written out, the g_dvdog
           test is the other way round with the arms swapped, and the
           stocking loop steps food_count in its own statements. */

void
er_food()
{
        short   food_count;
        short   roll;           /* declared, never written (link #-8) */

        g_actif = YES;
        wkFrDr();

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

        if (g_dvdog != NO) {
                sp_ssco(SPRITE_FOOD_PACKAGE);
                if (lcp_bwlS == BOWL_EMPTY) {
                        a_feedd(1);
                } else {
                        a_gesff();
                        g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
                        sp_upds();
                        g_lcyof = NO;
                }
        } else {
                sp_ssco(SPRITE_FOOD_PACKAGE);
                hs_posXY(POS_BTM_KITCHEN_CABINET,
                                      &g_wtx, &g_wty);
                lcp_wkD();

                g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
                sp_upds();
                g_lcyof = NO;
                lcp_face     = FACING_RIGHT;
                lcp_st                = STATE_STAND_FACING_SCREEN;
                g_hatas   = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();

                a_opecc(0);

                /* The flag is tested a second time -- redundant inside
                   this arm, but that is what the original does. */
                if (g_dvdog == NO) {
                        while (1) {
                                food_count =
                                        (lcp.door_states_and_flags >> 9) & 7;
                                food_count++;
                                if (food_count > 4)
                                        break;
                                food_count = food_count << 9;
                                lcp.door_states_and_flags &= ~DSF_FOOD_MASK;
                                lcp.door_states_and_flags |= food_count;
                                lcp_st = STATE_REACH_INTO_CABINET;
                                gameTick(3);
                                sc_drfc();
                                lcp_st = STATE_STAND_FACING_SCREEN;
                                gameTick(1);
                        }
                }

                if (lcp.initiative_threshold < rndRng(0, 100))
                        a_opecc(1);
                g_actif = NO;
        }
}
#endif
