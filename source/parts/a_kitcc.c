/*
 * parts/a_kitcc.c -- shared body; LCP_ORG links it in afood.o,
 * LCP_STX in the 0xdece object at 0x11354, far enough from
 * a_eatm that its call is a long bsr.  Files under parts/ are
 * never compiled standalone.
 */

/* a_kitcc: the eat routine.  Decrements food count, eats 10..20
   bite/chew cycles, resets hunger at end.  addr: a_kitcc() */

#ifdef FAITHFUL
void
a_kitcc()
{
        short           saved_head_frame;
        short           chew_delay;
        short           eat_cycles;
        short           inner;
        unsigned short  food_count;
        short           roll;

        pst_arr[0] = STATE_EAT_BITE;
        pst_arr[1] = STATE_EAT_CHEW;
        g_actif = YES;

        hs_posXY(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        lcp_wkD();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        a_opecc(0);

        food_count = (lcp.door_states_and_flags >> 9) & 7;
        if (food_count == 0) {
                gameTick(2);
                g_actif = NO;   /* ROM 0x13f6: cleared here too */
                return;
        }

        /* Decrement the 3-bit food-count nibble (bits 9..11). */
        lcp_st = STATE_REACH_INTO_CABINET;
        gameTick(3);
        lcp.door_states_and_flags =
                (lcp.door_states_and_flags & ~DSF_FOOD_MASK) |
                ((food_count - 1) * 0x200);
        sc_drfc();
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(2);

        roll = rndRng(0, 100);
        if (lcp.initiative_threshold < roll)
                a_opecc(1);

        sp_ssco(SPRITE_FOOD_PACKAGE);
        hs_posXY(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TABLE_SETTING);
        g_sepex[g_seslm[SPRITE_TABLE_SETTING]] = 103;
        g_sepey[g_seslm[SPRITE_TABLE_SETTING]] = 180;

        hs_posXY(POS_BTM_TABLE_RIGHT,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_TABLE_LEFT,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_hamod       = HEAD_ANIM_DISABLED;
        lcp_st            = STATE_STAND_SIDE_VIEW;
        lcp_face = FACING_RIGHT;
        sp_ss02(SPRITE_FOOD_PACKAGE);
        g_hatas = 8;
        lcp_hwt();

        saved_head_frame = g_hsfra;
        lcp_st        = pst_arr[0];
        lcp_y = lcp_y + 8;
        lcp_x = lcp_x + 6;
        eat_cycles       = rndRng(10, 20);
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;
        gameTick(0);
        g_lcyof = NO;
        g_sepex[g_seslm[SPRITE_FOOD_PACKAGE]] =
                g_sepex[g_seslm[SPRITE_FOOD_PACKAGE]] + 3;
        g_sepey[g_seslm[SPRITE_FOOD_PACKAGE]] =
                g_sepey[g_seslm[SPRITE_FOOD_PACKAGE]] - 4;
        gameTick(0);

        while (eat_cycles > 0) {
                lcp_st = pst_arr[1];
                gameTick(2);
                g_hsfra = 0;
                chew_delay = rndRng(1, 2);
                gameTick(chew_delay);
                lcp_st = pst_arr[0];
                g_hsfra = saved_head_frame;
                gameTick(0);

                inner = rndRng(4, 8);
                while (inner > 0 &&
                       g_trel[0] == ACTION_NONE) {
                        chew_delay = rndRng(1, 2);
                        gameTick(chew_delay);
                        g_hsfra = 1;
                        gameTick(0);
                        g_hsfra = 2;
                        gameTick(0);
                        inner = inner - 1;
                }
                g_hsfra = saved_head_frame;
                eat_cycles = eat_cycles - 1;
        }

        g_lcyof = YES;
        g_hatas   = 8;
        g_hacur        = 8;
        sp_ssco(SPRITE_FOOD_PACKAGE);
        lcp_y = lcp_y - 8;
        lcp_x = lcp_x - 6;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_hwt();
        gameTick(0);

        hs_posXY(POS_BTM_TABLE_RIGHT,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_FOOD_PACKAGE]  = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        gameTick(4);

        lcp.hunger_level   = NEED_SATISFIED;
        lcp.bathroom_timer = lcp.bathroom_timer_max;
        lcp_rcov();
        g_actif = NO;
}

#else   /* STX: link #-12 -- food_count, inner, eat_cycles,
           saved_head_frame; the chew delay and the initiative roll
           are consumed in place. */

void
a_kitcc()
{
        short   food_count;
        short   inner;
        short   eat_cycles;
        short   saved_head_frame;

        pst_arr[0] = STATE_EAT_BITE;
        pst_arr[1] = STATE_EAT_CHEW;
        g_actif = YES;

        hs_posXY(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        lcp_wkD();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        a_opecc(0);

        food_count = (lcp.door_states_and_flags >> 9) & 7;
        if (food_count == 0) {
                gameTick(2);
                return;
        }

        lcp_st = STATE_REACH_INTO_CABINET;
        gameTick(3);
        food_count--;
        lcp.door_states_and_flags =
                (food_count << 9) |
                (lcp.door_states_and_flags & ~DSF_FOOD_MASK);
        sc_drfc();
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(2);

        if (lcp.initiative_threshold < rndRng(0, 100))
                a_opecc(1);

        sp_ssco(SPRITE_FOOD_PACKAGE);
        hs_posXY(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_TABLE_SETTING);
        g_sepex[g_seslm[SPRITE_TABLE_SETTING]] = 103;
        g_sepey[g_seslm[SPRITE_TABLE_SETTING]] = 180;

        hs_posXY(POS_BTM_TABLE_RIGHT,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_TABLE_LEFT,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_hamod       = HEAD_ANIM_DISABLED;
        lcp_st            = STATE_STAND_SIDE_VIEW;
        lcp_face = FACING_RIGHT;
        sp_ss02(SPRITE_FOOD_PACKAGE);
        g_hatas = 8;
        lcp_hwt();

        lcp_st        = pst_arr[0];
        lcp_y += 8;
        lcp_x += 6;
        saved_head_frame = g_hsfra;
        eat_cycles       = rndRng(10, 20);
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;
        gameTick(0);
        g_lcyof = NO;
        g_sepex[g_seslm[SPRITE_FOOD_PACKAGE]] += 3;
        g_sepey[g_seslm[SPRITE_FOOD_PACKAGE]] -= 4;
        gameTick(0);

        while (eat_cycles-- > 0) {
                lcp_st = pst_arr[1];
                gameTick(2);
                g_hsfra = 0;
                gameTick(rndRng(1, 2));
                g_hsfra = saved_head_frame;
                lcp_st = pst_arr[0];
                gameTick(0);

                inner = rndRng(4, 8);
                while (inner-- > 0) {
                        if (g_trel[0] != ACTION_NONE)
                                break;
                        g_hsfra = saved_head_frame;
                        gameTick(rndRng(1, 2));
                        g_hsfra = 1;
                        gameTick(0);
                        g_hsfra = 2;
                        gameTick(0);
                }
                g_hsfra = saved_head_frame;
        }

        g_lcyof = YES;
        g_hatas   = 8;
        g_hacur        = 8;
        sp_ssco(SPRITE_FOOD_PACKAGE);
        lcp_y -= 8;
        lcp_x -= 6;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_hwt();
        gameTick(0);

        hs_posXY(POS_BTM_TABLE_RIGHT,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hs_posXY(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_selaf[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_FOOD_PACKAGE]  = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        gameTick(4);

        lcp.hunger_level   = NEED_SATISFIED;
        lcp.bathroom_timer = lcp.bathroom_timer_max;
        lcp_rcov();
        g_actif = NO;
}
#endif
