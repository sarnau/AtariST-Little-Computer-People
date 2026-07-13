/*
 * actions_doors.c -- door / cabinet / fridge / dresser open/close helpers.
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
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern short    lcp_closet_door_open;
extern short    lcp_dresser_open;
extern short    lcp_toilet_door_open;
extern short    lcp_filing_cabinet_open;
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
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    randomRange();                  /* random.c */
extern short    randomRange();
extern void     sf_sele();
extern void     object_draw();

/* a_clotd: 2-frame close animation.
   addr: a_clotd() */

void
a_clotd()
{
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        lcp_facing_direction = FACING_LEFT;
        lcp_state = STATE_BEND_AND_REACH;
        game_tick_and_animate(2);
        object_draw(g_obi09, 187, 87);
        game_tick_and_animate(2);
        object_draw(g_obidt, 187, 87);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        game_tick_and_animate(2);
        lcp_toilet_door_open = NO;

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* a_clocd: 2-frame close animation.
   addr: a_clocd() */

void
a_clocd()
{
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        lcp_facing_direction = FACING_LEFT;
        lcp_state = STATE_BEND_AND_REACH;
        game_tick_and_animate(2);
        object_draw(g_obi03, 75, 87);
        game_tick_and_animate(2);
        object_draw(g_obidc, 75, 87);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        game_tick_and_animate(2);
        lcp_closet_door_open = NO;

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* a_opecf: open, look inside, close.  Both SFX are
   SFX_DOOR_OPEN in the original -- preserved verbatim; whether the
   1985 source meant SFX_DOOR_CLOSE at the tail is a judgement call.
   addr: a_opecf() */

void
a_opecf()
{
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        lcp_facing_direction = FACING_LEFT;
        lcp_state = STATE_REACH_INTO_CABINET;
        object_draw(g_obi15, 24, 153);
        game_tick_and_animate(1);
        object_draw(g_obi16, 24, 153);
        sf_sele(SFX_DOOR_OPEN, 6L);
        game_tick_and_animate(1);
        object_draw(g_obi17, 24, 153);
        game_tick_and_animate(1);

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(2);

        lcp_facing_direction = FACING_LEFT;
        lcp_state = STATE_REACH_INTO_CABINET;
        game_tick_and_animate(3);

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(8);

        object_draw(g_obi16, 24, 153);
        game_tick_and_animate(1);
        object_draw(g_obi15, 24, 153);
        sf_sele(SFX_DOOR_OPEN, 6L);   /* verbatim */
        game_tick_and_animate(1);
}

/* a_opcfc: sequential open animation used by
   the write-letter and tidy-house flows.  Note that the original always
   ends with lcp_filing_cabinet_open = NO -- there's no "open" branch
   here; the cabinet is toggled elsewhere by walk_to_and_turn().
   addr: a_opcfc() */

void
a_opcfc()
{
        lcp_state = STATE_BEND_DOWN;         game_tick_and_animate(1);
        lcp_state = STATE_REACH_FORWARD;     game_tick_and_animate(2);
        lcp_state = STATE_PICK_UP_FROM_FLOOR;game_tick_and_animate(2);
        lcp_state = STATE_REACH_FORWARD;
        object_draw(g_obi13, 258, 47);
        game_tick_and_animate(1);
        lcp_state = STATE_BEND_DOWN;
        object_draw(g_obifc, 258, 47);
        game_tick_and_animate(1);
        lcp_filing_cabinet_open = NO;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* a_opecd: dual-mode open (value=0) / close (value=1)
   drawer with 2-frame sprite animation.
   addr: a_opecd() */

void
a_opecd(open_close_status)
short   open_close_status;
{
        if (open_close_status == 0) {
                if (lcp_dresser_open != NO)
                        return;
                lcp_dresser_open = YES;
                lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
                lcp_state = STATE_REACH_FORWARD;game_tick_and_animate(2);
                object_draw(g_obido, 97, 115);
                game_tick_and_animate(2);
                object_draw(g_obi12, 97, 115);
                game_tick_and_animate(2);
        } else {
                if (lcp_dresser_open == NO)
                        return;
                lcp_dresser_open = NO;
                lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
                lcp_state = STATE_REACH_FORWARD;game_tick_and_animate(2);
                object_draw(g_obido, 97, 115);
                game_tick_and_animate(2);
                object_draw(g_obi11, 97, 115);
                game_tick_and_animate(2);
        }
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* a_watat: filing-cabinet interaction helper -- opens
   the cabinet if closed, or reaches into it if already open, then
   nervously shifts facing direction 10 times.
   addr: a_watat() */

void
a_watat()
{
        short   i;

        lcp_state = STATE_BEND_DOWN;
        game_tick_and_animate(1);

        if (lcp_filing_cabinet_open == NO) {
                lcp_filing_cabinet_open = YES;
                lcp_state = STATE_REACH_FORWARD;
                object_draw(g_obi13, 258, 47);
                game_tick_and_animate(2);
                lcp_state = STATE_PICK_UP_FROM_FLOOR;
                object_draw(g_obi14, 258, 47);
                game_tick_and_animate(2);
        } else {
                lcp_state = STATE_REACH_FORWARD;
                game_tick_and_animate(1);
        }

        lcp_state = STATE_STOKE_FIREPLACE;
        game_tick_and_animate(1);
        for (i = 0; i < 10; i = i + 1) {
                lcp_facing_direction = randomRange(0, 1);
                game_tick_and_animate(0);
        }

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}
