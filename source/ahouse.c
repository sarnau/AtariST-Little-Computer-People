/*
 * ahouse.c -- walk-and-interact action handlers.
 *
 * Ports for actions that walk somewhere in the house, play an
 * interaction animation with SFX, and update world state.
 *
 * addr: a_readn(), a_gioob(),
 *       a_dance(), a_drink(), a_uset(),
 *       a_wakum(), a_gotbn()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>             /* Random() */
#include "abathrm.h"
#include "adoors.h"
#include "afood.h"
#include "ahouse.h"
#include "aleisure.h"
#include "asimple.h"
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


/* a_readn: armchair + TV + 200-frame reading loop.
   addr: a_readn() */

void
a_readn()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short           result;
#endif
#ifdef FAITHFUL
        unsigned short  rnd;
        short           t;
#else
        /* STX keeps the LIMIT at -2 and the counter at -4; it has no
           rnd local at all (the Random test is inline). */
        short           t;
        short           i;
#endif

        pst_arr[0] = STATE_READ_PAPER_HOLD;
        pst_arr[1] = STATE_READ_PAPER_TURN_PAGE;
        tt_on();
        hs_posXY(POS_TOP_ARMCHAIR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif

        g_hamod         = HEAD_ANIM_READING;
        lcp_face   = FACING_LEFT;
        lcp_st              = STATE_SIT_IN_ARMCHAIR;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER;
        lcp_hwt();
#ifdef FAITHFUL
        lcp_y = lcp_y + 8;
#else
        /* STX sets the counter before the coordinate steps and
           emits an addi #0 on lcp_x -- a no-op the original wrote. */
        t = 200;
        lcp_x += 0;
        lcp_y += 8;
        i = 0;
#endif

#ifdef FAITHFUL
        t = 0;
        while (t < 200 && g_trel[0] == ACTION_NONE) {
#else
        while (i < t) {
                if (g_trel[0] != ACTION_NONE)
                        break;
#endif
                lcp_face = FACING_LEFT;
                lcp_st            = pst_arr[0];
#ifdef FAITHFUL
                rnd = (unsigned short) Random();
                if ((rnd & 0xf) == 5)
#else
                if ((Random() & 0xf) == 5)
#endif
                        lcp_st = pst_arr[1];
                gameTick(1);
#ifdef FAITHFUL
                t = t + 1;
#else
                i++;
#endif
        }

#ifdef FAITHFUL
        lcp_y = lcp_y - 8;
#else
        lcp_y -= 8;
#endif
        lcp_face = FACING_LEFT;
        lcp_st = STATE_SIT_IN_ARMCHAIR;
        gameTick(2);
        tt_off();
}

/* a_gioob: undress and lie down, or reverse.
   addr: a_gioob() */

void
a_gioob()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short   result;
#endif

        pst_arr[0] = STATE_UNDRESS_AT_BED;
        pst_arr[1] = STATE_LIE_DOWN_GETTING_IN;
        pst_arr[2] = STATE_LIE_DOWN_IN_BED;

        if (lcp.is_sleeping == NO) {
                hs_posXY(POS_MID_BED,
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
                lcp_st              = STATE_STAND_IDLE;
                g_hatas = 10;
                lcp_hwt();
                lcp.is_sleeping = YES;
#ifdef FAITHFUL
                lcp_x = lcp_x - 10;
#else
                lcp_x -= 10;
#endif
                lcp_face = FACING_RIGHT;
                lcp_st = pst_arr[0]; gameTick(2);
#ifdef FAITHFUL
                lcp_x = lcp_x - 8;
#else
                lcp_x -= 8;
#endif
                lcp_st = pst_arr[1]; gameTick(2);
#ifdef FAITHFUL
                lcp_x = lcp_x - 2;
#else
                lcp_x -= 2;
#endif
                lcp_st = pst_arr[2]; gameTick(2);
        } else {
                lcp_face = FACING_RIGHT;
#ifdef FAITHFUL
                lcp_x = lcp_x + 10;
#else
                lcp_x += 10;
#endif
#ifdef FAITHFUL
                lcp_st = STATE_LIE_DOWN_GETTING_IN; gameTick(2);
#else
                lcp_st = pst_arr[1]; gameTick(2);
#endif
#ifdef FAITHFUL
                lcp_x = lcp_x + 10;
#else
                lcp_x += 10;
#endif
                lcp_st = pst_arr[0]; gameTick(2);
                lcp.is_sleeping = NO;
                lcp_st              = STATE_STAND_IDLE;
                g_hatas = 10;
                lcp_hwt();
                gameTick(2);
        }
}

/* a_dance: turn on the record player if needed, then step-shift
   until the song ends or the event queue interrupts.
   addr: a_dance() */

void
a_dance()
{
        /* STX has one local (the loop counter); the walk result is
           tested in place. */
#ifdef FAITHFUL
        short   result;
#endif
        short   i;

        pst_arr[0] = STATE_DANCE_STEP_LEFT;
        pst_arr[1] = STATE_DANCE_STEP_RIGHT;

        if (lcp_recP == NO) {
                g_actif = YES;
                a_lists();
        }
        g_actif = NO;

        hs_posXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        g_wty = g_wty + 8;
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        g_wty += 8;
        if (lcp_wkD() != 0)
                return;
#endif

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();

        /* STX never initialises i -- the first iteration reads
           whatever the frame slot held.  Preserved as written. */
#ifdef FAITHFUL
        i = 0;
        while (mi_play != NO) {
                i = i + 1;
#else
        while (mi_play != NO) {
                i++;
#endif
                lcp_st = pst_arr[i & 1];
                if (g_trel[0] != ACTION_NONE)
                        break;
                gameTick(2);
        }

        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}

/* a_drink: sink -> glass -> tap -> drink -> reset thirst.
   addr: a_drink() */

void
a_drink()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short   result;
#endif

        hs_posXY(POS_BTM_KITCHEN_SINK,
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

        g_actif = YES;
        sp_ssco(SPRITE_GLASS);
        hs_posXY(POS_BTM_WATER_TAP,
                              &g_wtx, &g_wty);
        lcp_wkD();

        g_selaf[SPRITE_GLASS] = SPRITE_HIDDEN;
        sp_upds();
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        if (lcp_watr != 0) {
                lcp_st = STATE_BEND_DOWN;
                lcp_face = FACING_RIGHT;
                gameTick(0);
                updWtLv(-3);
                g_hamod = HEAD_ANIM_DISABLED;
                lcp_st = STATE_DRINK_FROM_GLASS;
                gameTick(16);
                lcp_st = STATE_STAND_FACING_SCREEN;
#ifdef FAITHFUL
                lcp_y = lcp_y + 1;
#else
                lcp_y++;
#endif
                gameTick(3);
                a_driwa(3);
        }

        lcp.thirst_level = NEED_SATISFIED;
        lcp.thirst_timer = lcp.thirst_timer_max;
        lcp_rcov();
        g_selaf[SPRITE_GLASS] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        g_actif = NO;
}

/* a_uset: 3-sprite door animation, sit + flush + refill.
   addr: a_uset() */

void
a_uset()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short   result;
        short   saved_x;
        short   counter;
#else
        short   saved_x;
#endif

        hs_posXY(POS_MID_TOILET_DOOR,
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

        if (lcp_toiO == NO) {
                lcp_face = FACING_LEFT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                od_draw(od_tocl, 187, 87);
                gameTick(2);
                od_draw(od_too1, 187, 87);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(od_too2, 187, 87);
                gameTick(2);
                lcp_toiO = YES;
        }

        lcp_face = FACING_RIGHT;
        g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_3);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_3]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_3]] = 87;

        hs_posXY(POS_MID_TOILET_DOOR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        g_wty = g_wty - 3;
#else
        g_wty -= 3;
#endif
#ifdef FAITHFUL
        g_wtx = g_wtx - 10;
#else
        g_wtx -= 10;
#endif
        g_actif = YES;
        lcp_wkD();
        saved_x = lcp_x;

        /* Close door behind resident (3 sprite phases). */
        g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_2);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_2]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_2]] = 87;
        od_draw(od_too1, 187, 87);
        gameTick(1);

        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_1);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_1]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_1]] = 87;
        od_draw(od_tocl, 187, 87);
        hideLcp();
        sf_sele(SFX_DOOR_CLOSE, 6L);
        gameTick(1);

        /* 45..60 ticks, then flush + 16 tick refill. */
#ifdef FAITHFUL
        counter = rndRng(45, 60);
        gameTick(counter);
#else
        gameTick(rndRng(45, 60));       /* STX: no temporary */
#endif
        sf_sele(SFX_TOILET_FLUSH, 6L);
        gameTick(16);

        g_selaf[SPRITE_DOOR_ANIM_1] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_2);
        showLcp();
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_2]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_2]] = 87;
        od_draw(od_too1, 187, 87);
        sf_sele(SFX_DOOR_OPEN, 6L);
        gameTick(1);

        g_selaf[SPRITE_DOOR_ANIM_2] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_ANIM_3);
        g_sepex[g_seslm[SPRITE_DOOR_ANIM_3]] = 187;
        g_sepey[g_seslm[SPRITE_DOOR_ANIM_3]] = 87;
        od_draw(od_too2, 187, 87);
        gameTick(1);
        lcp_toiO = YES;

        lcp_x = saved_x;
        hs_posXY(POS_MID_TOILET_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();

        if (lcp_toiO != NO) {
                g_selaf[SPRITE_DOOR_ANIM_3] = SPRITE_HIDDEN;
                sp_upds();
                gameTick(0);
        }

#ifdef FAITHFUL
        counter = rndRng(0, 100);
        if (lcp.initiative_threshold < counter ||
            introSeq != NO)
#else
        if (lcp.initiative_threshold < rndRng(0, 100) ||
            introSeq != NO)
#endif
                a_clotd();

        lcp.bathroom_need  = NO;
        lcp.bathroom_timer = 9999;
        g_actif = NO;
}

/* a_wakum: scheduled morning routine.
   addr: a_wakum() */

void
a_wakum()
{
        /* STX has no local: the tick count is used in place. */
#ifdef FAITHFUL
        short   counter;
#endif

        g_actif = YES;
        alarm_p = YES;
#ifdef FAITHFUL
        counter = rndRng(40, 100);
        gameTick(counter);
#else
        gameTick(rndRng(40, 100));
#endif
        if (lcp.is_sleeping == YES)
                a_gioob();

        g_actif = YES; a_wakfa();
        g_actif = YES; a_takes();
        g_actif = YES; a_brust();
        g_actif = YES; a_opcbc(0);
        g_actif = YES; a_eatm();
        g_actif = NO;
}

/* a_gotbn: scheduled bedtime routine.
   addr: a_gotbn() */

void
a_gotbn()
{
        g_actif = YES; a_takes();
        g_actif = YES; a_opcbc(1);
        g_actif = YES; a_kitcc();
        g_actif = YES; a_brust();
        g_actif = YES; a_gioob();
        g_actif = NO;
}

/* a_getd: pure head-anim routine.  Turns the head to face
   a canonical resting direction, then oscillates the vertical tilt bit
   four times (undressing / dressing motion communicated via head bob).
   No walking, no world state change.
   addr: a_getd() */

void
a_getd()
{
        short   entry_current;
        short   h;
        /* STX has only two locals: it reuses h as the loop counter
           below, so its frame is 2 bytes smaller. */
#ifdef FAITHFUL
        short   i;
#endif

        entry_current = g_hacur;
        h = g_hacur & 7;

        if (h == 0 || h == 1 || h == 7)
                g_hatas = 8;
        else if (h == 2)                        /* HEAD_ANIM_SHOWER value */
                g_hatas = 9;
        else if (h == 6)
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE |
                                         7 /* HEAD_MODE_H_AMPLITUDE mask */;
        else if (h == 3 || h == 4)
                g_hatas = 10;
        else if (h == 5)
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE |
                                         HEAD_ANIM_SHOWER;

        g_hamod = HEAD_ANIM_DISABLED;
        lcp_hwt();

#ifdef FAITHFUL
        for (i = 0; i < 4; i = i + 1) {
#else
        for (h = 0; h < 4; h++) {
#endif
                g_hatas = g_hacur & 7;
                lcp_hwt();
                g_hatas = g_hacur | 0x10;
                lcp_hwt();
        }

        g_hatas = entry_current;
        lcp_hwt();
}

/* li_lool / li_loor: the two 4-tick "stand-and-
   look" gestures used by the TV toggle, record player, and post-action
   idle transitions.  The 1985 code sets FACING_RIGHT in both -- the
   "left" / "right" naming refers to which head-frame direction the
   animation actually plays via g_hatas, not the body
   facing.  Preserved verbatim.
   addr: li_lool(), li_loor() */

void
li_lool()
{
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        lcp_st = STATE_BEND_DOWN;
        gameTick(4);
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}

void
li_loor()
{
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        lcp_st = STATE_BEND_DOWN;
        gameTick(4);
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
