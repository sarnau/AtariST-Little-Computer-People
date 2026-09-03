/*
 * parts/er_food.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * delivery functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */


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
