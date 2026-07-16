/*
 * adoors.c -- door / cabinet / fridge / dresser open/close helpers.
 *
 * These are the small "close it" companions to the larger walk-and-
 * interact handlers.  They assume the resident is already at the
 * correct HOUSE_POS -- callers walk first, then call these to play the
 * animation + toggle the runtime flag + emit the SFX.
 *
 * addr: a_clotd(), a_clocd(),
 *       a_opecf(), a_opcfc(),
 *       a_opecd(), a_watat()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    g_hatas;
extern void     lcp_hwt();
extern void     gameTick();
extern short    lcp_clsO;
extern short    lcp_drsO;
extern short    lcp_toiO;
extern short    lcp_flcO;
extern short    g_obidt;
extern short    g_obi09;
extern short    g_obi15;
extern short    g_obi16;
extern short    g_obi17;
extern short    g_obidc;
extern short    g_obi03;
extern short    g_obifc;
extern short    g_obi13;
extern short    g_obi14;
extern short    g_obi11;
extern short    g_obido;
extern short    g_obi12;
extern short    lcp_st;
extern short    lcp_face;
extern short    rndRng();                  /* random.c */
extern short    rndRng();
extern void     sf_sele();
extern void     od_draw();

/* a_clotd: 2-frame close animation.
   addr: a_clotd() */

void
a_clotd()
{
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        lcp_face = FACING_LEFT;
        lcp_st = STATE_BEND_AND_REACH;
        gameTick(2);
        od_draw(g_obi09, 187, 87);
        gameTick(2);
        od_draw(g_obidt, 187, 87);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        lcp_toiO = NO;

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}

/* a_clocd: 2-frame close animation.
   addr: a_clocd() */

void
a_clocd()
{
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        lcp_face = FACING_LEFT;
        lcp_st = STATE_BEND_AND_REACH;
        gameTick(2);
        od_draw(g_obi03, 75, 87);
        gameTick(2);
        od_draw(g_obidc, 75, 87);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        lcp_clsO = NO;

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}

/* a_opecf: open, look inside, close.  Both SFX are
   SFX_DOOR_OPEN in the original -- preserved verbatim; whether the
   1985 source meant SFX_DOOR_CLOSE at the tail is a judgement call.
   addr: a_opecf() */

void
a_opecf()
{
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        lcp_face = FACING_LEFT;
        lcp_st = STATE_REACH_INTO_CABINET;
        od_draw(g_obi15, 24, 153);
        gameTick(1);
        od_draw(g_obi16, 24, 153);
        sf_sele(SFX_DOOR_OPEN, 6L);
        gameTick(1);
        od_draw(g_obi17, 24, 153);
        gameTick(1);

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(2);

        lcp_face = FACING_LEFT;
        lcp_st = STATE_REACH_INTO_CABINET;
        gameTick(3);

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(8);

        od_draw(g_obi16, 24, 153);
        gameTick(1);
        od_draw(g_obi15, 24, 153);
        sf_sele(SFX_DOOR_OPEN, 6L);   /* verbatim */
        gameTick(1);
}

/* a_opcfc: sequential open animation used by
   the write-letter and tidy-house flows.  Note that the original always
   ends with lcp_flcO = NO -- there's no "open" branch
   here; the cabinet is toggled elsewhere by walk_to_and_turn().
   addr: a_opcfc() */

void
a_opcfc()
{
        lcp_st = STATE_BEND_DOWN;         gameTick(1);
        lcp_st = STATE_REACH_FORWARD;     gameTick(2);
        lcp_st = STATE_PICK_UP_FROM_FLOOR;gameTick(2);
        lcp_st = STATE_REACH_FORWARD;
        od_draw(g_obi13, 258, 47);
        gameTick(1);
        lcp_st = STATE_BEND_DOWN;
        od_draw(g_obifc, 258, 47);
        gameTick(1);
        lcp_flcO = NO;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}

/* a_opecd: dual-mode open (value=0) / close (value=1)
   drawer with 2-frame sprite animation.
   addr: a_opecd() */

void
a_opecd(oc_stat)
short   oc_stat;
{
        if (oc_stat == 0) {
                if (lcp_drsO != NO)
                        return;
                lcp_drsO = YES;
                lcp_st = STATE_BEND_DOWN;    gameTick(1);
                lcp_st = STATE_REACH_FORWARD;gameTick(2);
                od_draw(g_obido, 97, 115);
                gameTick(2);
                od_draw(g_obi12, 97, 115);
                gameTick(2);
        } else {
                if (lcp_drsO == NO)
                        return;
                lcp_drsO = NO;
                lcp_st = STATE_BEND_DOWN;    gameTick(1);
                lcp_st = STATE_REACH_FORWARD;gameTick(2);
                od_draw(g_obido, 97, 115);
                gameTick(2);
                od_draw(g_obi11, 97, 115);
                gameTick(2);
        }
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}

/* a_watat: filing-cabinet interaction helper -- opens
   the cabinet if closed, or reaches into it if already open, then
   nervously shifts facing direction 10 times.
   addr: a_watat() */

void
a_watat()
{
        short   i;

        lcp_st = STATE_BEND_DOWN;
        gameTick(1);

        if (lcp_flcO == NO) {
                lcp_flcO = YES;
                lcp_st = STATE_REACH_FORWARD;
                od_draw(g_obi13, 258, 47);
                gameTick(2);
                lcp_st = STATE_PICK_UP_FROM_FLOOR;
                od_draw(g_obi14, 258, 47);
                gameTick(2);
        } else {
                lcp_st = STATE_REACH_FORWARD;
                gameTick(1);
        }

        lcp_st = STATE_STOKE_FIREPLACE;
        gameTick(1);
        for (i = 0; i < 10; i = i + 1) {
                lcp_face = rndRng(0, 1);
                gameTick(0);
        }

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
