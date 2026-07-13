/*
 * actions_doors.c -- door / cabinet / fridge / dresser open/close helpers.
 *
 * These are the small "close it" companions to the larger walk-and-
 * interact handlers.  They assume the resident is already at the
 * correct HOUSE_POS -- callers walk first, then call these to play the
 * animation + toggle the runtime flag + emit the SFX.
 *
 * addr: action_close_toilet_door(), action_close_closet_door(),
 *       action_open_close_fridge(), action_open_close_filing_cabinet(),
 *       action_open_close_dresser(), action_walk_to_and_turn()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"

extern short    randomRange();
extern void     soundeffect_select();
extern void     object_draw();

/* action_close_toilet_door: 2-frame close animation.
   addr: action_close_toilet_door() */

void
action_close_toilet_door()
{
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        lcp_facing_direction = FACING_LEFT;
        lcp_state = STATE_BEND_AND_REACH;
        game_tick_and_animate(2);
        object_draw(object_id_door_toilet_open_1, 187, 87);
        game_tick_and_animate(2);
        object_draw(object_id_door_toilet_closed, 187, 87);
        soundeffect_select(SFX_DOOR_CLOSE, 6L);
        game_tick_and_animate(2);
        lcp_toilet_door_open = NO;

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* action_close_closet_door: 2-frame close animation.
   addr: action_close_closet_door() */

void
action_close_closet_door()
{
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        lcp_facing_direction = FACING_LEFT;
        lcp_state = STATE_BEND_AND_REACH;
        game_tick_and_animate(2);
        object_draw(object_id_door_closet_open_1, 75, 87);
        game_tick_and_animate(2);
        object_draw(object_id_door_closet_closed, 75, 87);
        soundeffect_select(SFX_DOOR_CLOSE, 6L);
        game_tick_and_animate(2);
        lcp_closet_door_open = NO;

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* action_open_close_fridge: open, look inside, close.  Both SFX are
   SFX_DOOR_OPEN in the original -- preserved verbatim; whether the
   1985 source meant SFX_DOOR_CLOSE at the tail is a judgement call.
   addr: action_open_close_fridge() */

void
action_open_close_fridge()
{
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        lcp_facing_direction = FACING_LEFT;
        lcp_state = STATE_REACH_INTO_CABINET;
        object_draw(object_id_fridge_closed, 24, 153);
        game_tick_and_animate(1);
        object_draw(object_id_fridge_open_1, 24, 153);
        soundeffect_select(SFX_DOOR_OPEN, 6L);
        game_tick_and_animate(1);
        object_draw(object_id_fridge_open_2, 24, 153);
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

        object_draw(object_id_fridge_open_1, 24, 153);
        game_tick_and_animate(1);
        object_draw(object_id_fridge_closed, 24, 153);
        soundeffect_select(SFX_DOOR_OPEN, 6L);   /* verbatim */
        game_tick_and_animate(1);
}

/* action_open_close_filing_cabinet: sequential open animation used by
   the write-letter and tidy-house flows.  Note that the original always
   ends with lcp_filing_cabinet_open = NO -- there's no "open" branch
   here; the cabinet is toggled elsewhere by walk_to_and_turn().
   addr: action_open_close_filing_cabinet() */

void
action_open_close_filing_cabinet()
{
        lcp_state = STATE_BEND_DOWN;         game_tick_and_animate(1);
        lcp_state = STATE_REACH_FORWARD;     game_tick_and_animate(2);
        lcp_state = STATE_PICK_UP_FROM_FLOOR;game_tick_and_animate(2);
        lcp_state = STATE_REACH_FORWARD;
        object_draw(object_id_filing_cabinet_open_1, 258, 47);
        game_tick_and_animate(1);
        lcp_state = STATE_BEND_DOWN;
        object_draw(object_id_filing_cabinet_closed, 258, 47);
        game_tick_and_animate(1);
        lcp_filing_cabinet_open = NO;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* action_open_close_dresser: dual-mode open (value=0) / close (value=1)
   drawer with 2-frame sprite animation.
   addr: action_open_close_dresser() */

void
action_open_close_dresser(open_close_status)
short   open_close_status;
{
        if (open_close_status == 0) {
                if (lcp_dresser_open != NO)
                        return;
                lcp_dresser_open = YES;
                lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
                lcp_state = STATE_REACH_FORWARD;game_tick_and_animate(2);
                object_draw(object_id_dresser_open_1, 97, 115);
                game_tick_and_animate(2);
                object_draw(object_id_dresser_open_2, 97, 115);
                game_tick_and_animate(2);
        } else {
                if (lcp_dresser_open == NO)
                        return;
                lcp_dresser_open = NO;
                lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
                lcp_state = STATE_REACH_FORWARD;game_tick_and_animate(2);
                object_draw(object_id_dresser_open_1, 97, 115);
                game_tick_and_animate(2);
                object_draw(object_id_dresser_closed, 97, 115);
                game_tick_and_animate(2);
        }
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* action_walk_to_and_turn: filing-cabinet interaction helper -- opens
   the cabinet if closed, or reaches into it if already open, then
   nervously shifts facing direction 10 times.
   addr: action_walk_to_and_turn() */

void
action_walk_to_and_turn()
{
        short   i;

        lcp_state = STATE_BEND_DOWN;
        game_tick_and_animate(1);

        if (lcp_filing_cabinet_open == NO) {
                lcp_filing_cabinet_open = YES;
                lcp_state = STATE_REACH_FORWARD;
                object_draw(object_id_filing_cabinet_open_1, 258, 47);
                game_tick_and_animate(2);
                lcp_state = STATE_PICK_UP_FROM_FLOOR;
                object_draw(object_id_filing_cabinet_open_2, 258, 47);
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
