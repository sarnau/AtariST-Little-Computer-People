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

/* a_kitcc -> parts/a_kitcc.c (STX: 0xdece object, 0x11354). */
#ifdef FAITHFUL
#include "parts/a_kitcc.c"
#endif

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
