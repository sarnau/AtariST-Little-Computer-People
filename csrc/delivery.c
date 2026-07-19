/*
 * delivery.c -- Ctrl+F/B/R/D/C doorbell events.
 *
 * All five deferred events share the same open-door-pick-up pattern:
 *   1. wkFrDr
 *   2. face right, stand facing screen, look forward
 *   3. open front door (unless already open)
 *   4. bend down, reach forward, bend down again (the "pickup")
 *   5. maybe close the door (initiative_threshold roll)
 *   6. attach a carried sprite and walk to the destination shelf
 *   7. bend down / reach forward again to put it down
 *
 * The Ctrl+D dog-food variant reuses er_food with
 * g_dvdog=YES so the food goes to the dog bowl instead of
 * the kitchen cabinet.
 *
 * ev_ansPh is grouped here because it's the same event-queue
 * consumer even though it's a phone call rather than a delivery.
 *
 * addr: er_food(), er_bood(),
 *       er_recd(), er_dogf(),
 *       ev_ansPh(), wkFrDr(),
 *       a_opcfd(), a_opecc()
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


/* wkFrDr: tiny helper used by all four delivery events.
   addr: wkFrDr() */

void
wkFrDr()
{
        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();
}

/* a_opcfd: toggle the front door with SFX+draw.
   Called from every delivery event and from event handlers via the
   initiative-threshold roll.  door_st=0 opens, 1 closes.
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
                od_draw(g_obi05, 294, 151);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(g_obi06, 294, 151);
                gameTick(2);
                lcp_frdO = YES;
        } else {
                if (lcp_frdO == NO)
                        return;
                od_draw(g_obi05, 294, 151);
                gameTick(2);
                od_draw(g_obidf, 294, 151);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                gameTick(2);
                lcp_frdO = NO;
        }
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}

/* a_opecc: kitchen cabinet toggle used by the food
   delivery to reveal / hide the stocked cabinet interior.
   addr: a_opecc() */

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
                od_draw(g_obico, 46, 140);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(g_obi02, 46, 140);
                sc_drfc();
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(2);
        } else if (lcp_cabO != NO) {
                lcp_cabO = NO;
                lcp_st = STATE_REACH_INTO_CABINET;
                gameTick(3);
                od_draw(g_obico, 46, 140);
                gameTick(2);
                od_draw(g_obicc, 46, 140);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(2);
        }
}

/* Small helper: the "at the door, pick up" sequence common to all
   deliveries: face right + look forward, open door, bend / reach /
   bend / stand, then optionally close the door based on the
   initiative-threshold roll. */

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

/* er_food: Ctrl+F grocery event.  Also reused by
   er_dogf with g_dvdog set.
   addr: er_food() */

void
er_food()
{
        unsigned short  food_count;
        short           roll;

        g_actif = YES;
        wkFrDr();
        dv_pick();

        if (g_dvdog == NO) {
                sp_ssco(SPRITE_FOOD_PACKAGE);
                hs_posXY(POS_BTM_KITCHEN_CABINET,
                                      &g_wtx, &g_wty);
                lcp_wkD();

                g_selaf[9] = SPRITE_HIDDEN;
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
                        g_selaf[9] = SPRITE_HIDDEN;
                        sp_upds();
                        g_lcyof = NO;
                }
        }
}

/* er_bood: Ctrl+B.  Book -> bookshelf.
   addr: er_bood() */

void
er_bood()
{
        g_actif = YES;
        wkFrDr();
        dv_pick();

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
        g_actif = YES;
        wkFrDr();
        dv_pick();

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

        lcp_food = lcp_food + 1;    /* 1985 typo, preserved */
        g_actif = NO;
}

/* er_dogf: Ctrl+D.  Trivial trampoline into food
   delivery with g_dvdog set.
   addr: er_dogf() */

void
er_dogf()
{
        g_dvdog = YES;
        er_food();
        g_dvdog = NO;
}

/* ev_ansPh: Ctrl+C or random daytime call.  Calls the dog
   over first (which puts the resident at position 43 next to the
   phone), then picks up the handset, talks for 40..50 ticks with
   random head positions and speech SFX, hangs up, crouches, waits for
   any petting to finish, then stands.  ph_ans prevents
   re-entry while the animation is running.

   addr: ev_ansPh() */

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

        lcp_y = lcp_y + 6;
        lcp_st = STATE_PHONE_PICKUP;
        gameTick(1);

        ph_ans    = YES;
        ph_call = NO;
        ph_hu      = YES;
        gameTick(0);
        od_draw(g_obipc, 190, 168);

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
                ticks = ticks - 1;
        }

        ph_hu = YES;
        lcp_st         = STATE_PHONE_PICKUP;
        g_hsfra = saved_frame;
        gameTick(1);

        lcp_y = lcp_y - 6;
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(1);

        while (g_ptdoa != NO)
                gameTick(0);

        dg_petok = NO;
        lcp_y = lcp_y - 2;
        g_hatas = 8;
        g_hacur      = 8;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_hwt();
        gameTick(0);
        ph_ans = NO;
}
