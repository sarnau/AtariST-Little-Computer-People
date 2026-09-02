/*
 * delivery.c -- Ctrl+F/B/R/D/C doorbell events.
 * Shared shape: wkFrDr, open door, bend/reach/bend, maybe close,
 * attach carried sprite, walk to shelf, put down.
 * Ctrl+D dog-food reuses er_food with g_dvdog=YES.
 * ev_ansPh is here because it's the same event-queue consumer.
 * addr: er_food(), er_bood(), er_recd(), er_dogf(), ev_ansPh(),
 *       wkFrDr(), a_opcfd(), a_opecc()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "afood.h"
#include "asimple.h"
#include "delivery.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"


/* addr: wkFrDr() */

void
wkFrDr()
{
        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();
}

/* a_opcfd: toggle the front door.  door_st=0 opens, 1 closes.
   addr: a_opcfd() */

void
a_opcfd(door_st)
short   door_st;
{
        if (door_st == 0) {
                if (lcp_frdO != NO)
                        return;
                lcp_face = FACING_RIGHT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                od_draw(od_fro1, 294, 151);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(od_fro2, 294, 151);
                gameTick(2);
                lcp_frdO = YES;
        } else {
                if (lcp_frdO == NO)
                        return;
                od_draw(od_fro1, 294, 151);
                gameTick(2);
                od_draw(od_frcl, 294, 151);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                gameTick(2);
                lcp_frdO = NO;
        }
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}

/* a_opecc: kitchen cabinet toggle.  addr: a_opecc() */

void
a_opecc(oc_stat)
short   oc_stat;
{
        if (oc_stat == 0) {
                if (lcp_cabO != NO)
                        return;
                lcp_cabO = YES;
                lcp_st = STATE_REACH_INTO_CABINET;
                gameTick(3);
                od_draw(od_cbo1, 46, 140);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(od_cbo2, 46, 140);
                sc_drfc();
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(2);
        } else if (lcp_cabO != NO) {
                lcp_cabO = NO;
                lcp_st = STATE_REACH_INTO_CABINET;
                gameTick(3);
                od_draw(od_cbo1, 46, 140);
                gameTick(2);
                od_draw(od_cbcl, 46, 140);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(2);
        }
}

/* Common "at the door, pick up" sequence: open door, bend/reach/bend,
   then optionally close via the initiative-threshold roll. */

#ifdef FAITHFUL
static void
dv_pick()
{
        short   roll;

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

        roll = rndRng(0, 100);
        if (lcp.initiative_threshold < roll)
                a_opcfd(1);
}
#endif

/* er_food: Ctrl+F grocery event.  Reused by er_dogf with g_dvdog set.
   addr: er_food() */

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

/* er_bood: Ctrl+B.  Book -> bookshelf.  addr: er_bood() */

void
er_bood()
{
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

/* er_recd: Ctrl+R.  Record -> dance floor shelf.
   Note the original also increments lcp_food at the end -- this
   looks like an off-by-one bug (should have been counting records), but
   preserved for faithfulness.
   addr: er_recd() */

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

/* er_dogf: Ctrl+D.  Trampoline into er_food with g_dvdog set.
   addr: er_dogf() */

void
er_dogf()
{
        g_dvdog = YES;
        er_food();
        g_dvdog = NO;
}

/* ev_ansPh: Ctrl+C or random daytime call.  a_calld() puts the resident
   at the phone (position 43); talks 40..50 ticks with random head
   positions/SFX; ph_ans guards re-entry.  addr: ev_ansPh() */

#ifdef FAITHFUL
void
ev_ansPh()
{
        short   pick;
        short   saved_frame;
        short   ticks;
        short   subpick;

        g_actif = YES;
        a_calld();
        g_actif = NO;

        g_hamod         = HEAD_ANIM_DISABLED;
        g_hatas = 8;
        lcp_hwt();

#ifdef FAITHFUL
        lcp_y = lcp_y + 6;
#else
        lcp_y += 6;
#endif
        lcp_st = STATE_PHONE_PICKUP;
        gameTick(1);

        ph_ans    = YES;
        ph_call = NO;
        ph_hu      = YES;
        gameTick(0);
        /* ROM reads od_med1 (=40, OBJ_MEDICINE_OPEN_1) here, not the
           phone-call frame -- likely an original-game slip, kept. */
        od_draw(od_med1, 190, 168);

        lcp_st = STATE_PHONE_TALKING;
        gameTick(1);

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        ticks = rndRng(0x28, 0x32);
        while (ticks != 0) {
                pick = rndRng(0, 2);
                if (pick == 0) {
                        g_hsfra = 5;
                        p_sftvc();
                } else if (pick == 1) {
                        g_hsfra = 6;
                        subpick = rndRng(0, 1);
                        if (subpick == 0)
                                p_sfgrt();
                        else
                                p_sfspe();
                } else {
                        g_hsfra = saved_frame;
                        p_sfhnd();
                }
                subpick = rndRng(1, 2);
                gameTick(subpick);
                g_sfret = (long) subpick;
#ifdef FAITHFUL
                ticks = ticks - 1;
#else
                ticks -= 1;
#endif
        }

        ph_hu = YES;
        lcp_st         = STATE_PHONE_PICKUP;
        g_hsfra = saved_frame;
        gameTick(1);

#ifdef FAITHFUL
        lcp_y = lcp_y - 6;
#else
        lcp_y -= 6;
#endif
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(1);

        while (g_ptdoa != NO)
                gameTick(0);

        dg_petok = NO;
#ifdef FAITHFUL
        lcp_y = lcp_y - 2;
#else
        lcp_y -= 2;
#endif
        g_hatas = 8;
        g_hacur      = 8;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_hwt();
        gameTick(0);
        ph_ans = NO;
}
#else   /* STX: link #-10 -- saved_frame, ticks, subpick; the
           gesture pick is a switch, not an if/else ladder. */

void
ev_ansPh()
{
        short   saved_frame;
        short   ticks;
        short   subpick;

        g_actif = YES;
        a_calld();
        g_actif = NO;

        g_hamod         = HEAD_ANIM_DISABLED;
        g_hatas = 8;
        lcp_hwt();

        lcp_y += 6;
        lcp_st = STATE_PHONE_PICKUP;
        gameTick(1);

        ph_ans    = YES;
        ph_call = NO;
        ph_hu      = YES;
        gameTick(0);
        od_draw(od_med1, 190, 168);

        lcp_st = STATE_PHONE_TALKING;
        gameTick(1);

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        ticks = rndRng(0x28, 0x32);
        while (ticks-- != 0) {
                switch (rndRng(0, 2)) {
                case 0:
                        g_hsfra = 5;
                        p_sftvc();
                        break;
                case 1:
                        g_hsfra = 6;
                        if (rndRng(0, 1) != 0)
                                p_sfspe();
                        else
                                p_sfgrt();
                        break;
                case 2:
                        g_hsfra = saved_frame;
                        p_sfhnd();
                        break;
                }
                gameTick(subpick = rndRng(1, 2));
                g_sfret = (long) subpick;
        }

        g_hsfra = saved_frame;
        ph_hu = YES;
        lcp_st         = STATE_PHONE_PICKUP;
        gameTick(1);

        lcp_y -= 6;
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(1);

        while (g_ptdoa != NO)
                gameTick(0);

        dg_petok = NO;
        lcp_y -= 2;
        g_hatas = 8;
        g_hacur      = 8;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_hwt();
        gameTick(0);
        ph_ans = NO;
}
#endif
