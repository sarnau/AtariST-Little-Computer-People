/*
 * actions_idle.c -- short "no-walk" idle / gesture handlers.
 *
 * All share the same shape: pick a pair of animation states, tick
 * through them for a short duration, return to STATE_STAND_SIDE_VIEW.
 * No walking, no world state mutation, no sound (except toggle_tv).
 *
 * addr: action_wander_idly(), action_peek_around(),
 *       action_pace_nervously(), action_toggle_tv(), action_sleep()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     soundeffect_select();
extern void     tv_turn_on();
extern void     tv_turn_off();

/* action_wander_idly: two-state shrug idle.
   addr: action_wander_idly() */

void
action_wander_idly()
{
        PLAYER_STATE_ARRAY[0]  = STATE_IDLE_SHRUG_START;
        PLAYER_STATE_ARRAY[1]  = STATE_IDLE_SHRUG_HOLD;
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();

        lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(2);
        lcp_state = PLAYER_STATE_ARRAY[1]; game_tick_and_animate(5);
        lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(2);
        lcp_state = STATE_STAND_SIDE_VIEW; game_tick_and_animate(0);
}

/* action_peek_around: 6-tick look-away with head frame 2.
   addr: action_peek_around() */

void
action_peek_around()
{
        short   saved_frame;

        head_anim_target_state = 8;
        head_anim_mode         = HEAD_ANIM_DISABLED;
        lcp_wait_head_reach_target();

        saved_frame            = head_sprite_frame;
        head_anim_target_state = HEAD_ANIM_DISABLED;
        head_anim_current      = HEAD_ANIM_DISABLED;
        head_sprite_frame      = 2;
        game_tick_and_animate(6);

        head_anim_target_state = 8;
        head_anim_current      = 8;
        head_sprite_frame      = saved_frame;
        game_tick_and_animate(0);
}

/* action_pace_nervously: 15-frame side-shift alternation.
   addr: action_pace_nervously() */

void
action_pace_nervously()
{
        short   i;

        PLAYER_STATE_ARRAY[0]  = STATE_PACE_SHIFT_LEFT;
        PLAYER_STATE_ARRAY[1]  = STATE_PACE_SHIFT_RIGHT;
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();

        for (i = 0; i < 15; i = i + 1) {
                lcp_state = PLAYER_STATE_ARRAY[i & 1];
                game_tick_and_animate(1);
        }
        lcp_state = STATE_STAND_SIDE_VIEW;
        game_tick_and_animate(0);
}

/* action_toggle_tv: flip the TV state.  Both tv_turn_on and tv_turn_off
   handle their own SFX_TV_CLICK.
   addr: action_toggle_tv() */

void
action_toggle_tv()
{
        if (lcp_tv_on == NO)
                tv_turn_on();
        else
                tv_turn_off();
}

/* action_sleep: lie in bed, snore, optionally forever (value == -1 is
   the copy-protection punishment path).  On value == -1 the resident
   first walks to the current floor's center Y before lying down.
   addr: action_sleep() */

void
action_sleep(value)
short   value;
{
        short   duration;
        short   i;
        short   floor;

        PLAYER_STATE_ARRAY[0] = STATE_SLEEP_BREATHE_IN;
        PLAYER_STATE_ARRAY[1] = STATE_SLEEP_BREATHE_OUT;

        if (lcp_on_stairs_flag != NO)
                return;

        if (value == -1) {
                walk_target_x = lcp_x;
                floor = get_floor_number_from_y(lcp_y);
                walk_target_y = floor_center_y_coords[floor - 1];
                if (lcp_walk_to_destination() != 0)
                        return;
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_SIDE_VIEW;
                head_anim_target_state = 8;
                lcp_wait_head_reach_target();
        }

        duration = randomRange(7, 15);
        if (value != -1)
                duration = value;

        i = 0;
        while (i < duration &&
               triggered_event_list[0] == ACTION_NONE) {
                lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(1);
                lcp_state = PLAYER_STATE_ARRAY[1]; game_tick_and_animate(0);
                soundeffect_select(SFX_SNORING, 3L);
                game_tick_and_animate(1);
                lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(1);
                i = i + 1;
        }

        if (value == -1) {
                lcp_state = STATE_STAND_SIDE_VIEW;
                game_tick_and_animate(0);
        }
}
