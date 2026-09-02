/*
 * afood.c -- meal, kitchen, feed-dog, snack handlers.
 *
 * All four share the kitchen-cabinet / fridge / stove workflow and
 * update food-supply / hunger / dog-bowl state at their tail.
 *
 * addr: a_eatm(), a_kitcc(),
 *       a_feedd(), a_gesff()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "adoors.h"
#include "afood.h"
#include "delivery.h"
#include "events.h"
#include "globals.h"
#include "health.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"

/* a_eatm: pot from cabinet -> stove (cooking anim) -> chains into
   a_kitcc() to eat.  addr: a_eatm() */

void
a_eatm()
{
        short   result;
        short   counter;
        short   pick;

        hs_posXY(POS_BTM_KITCHEN_CABINET,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

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
        counter = rndRng(30, 50);
        while (counter != 0) {
                pick = rndRng(0, 2);
                od_draw(g_obisa[pick], 6, 172);
                gameTick(1);
                counter = counter - 1;
        }
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

/* a_kitcc: the eat routine.  Decrements food count, eats 10..20
   bite/chew cycles, resets hunger at end.  addr: a_kitcc() */

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

/* a_feedd: fridge -> dog bowl -> fridge.
   value==0: standalone, open fridge first.
   value==1: Ctrl+D delivery path, package already in hand.
   addr: a_feedd() */

void
a_feedd(value)
short   value;
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short   result;
#endif

        if (value == 0) {
                hs_posXY(POS_BTM_FRIDGE,
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

/* a_gesff -> parts/a_gesff.c (STX places it between a_clocd
   and a_opecf in the 0xdece object). */
#ifdef FAITHFUL
#include "parts/a_gesff.c"
#endif
