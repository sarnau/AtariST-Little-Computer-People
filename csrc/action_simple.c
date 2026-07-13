/*
 * action_simple.c -- short idle / gesture actions.
 *
 * Ports for do_action() handlers that don't need walking to a specific
 * house position and involve mostly head/body animation with sound.
 *
 * addr: action_wake_from_alarm(), action_hello(), action_yawn_and_stretch(),
 *       action_nod_head(), action_pet_dog(), action_call_dog()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     play_soundeffect_tv_click();
extern void     play_soundeffect_greeting();
extern void     play_soundeffect_speech();
extern void     play_soundeffect_head_nod();
extern void     soundeffects_off();

/* action_wake_from_alarm: Ctrl+A path.  Walks to the bedroom alarm,
   faces right, silences the alarm and clears the pressed flag.
   addr: action_wake_from_alarm() */

void
action_wake_from_alarm()
{
        short   result;

        house_get_position_xy(POS_MID_BEDROOM_WALK,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result == 0) {
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
                ctrl_a_alarm_pressed_flag = NO;
        }
}

/* action_hello: face-forward wave with a random 20-40 head sequence.
   Interruptible via the deferred-event queue.
   addr: action_hello() */

void
action_hello()
{
        short   wave_count;
        short   pick;
        short   prev_pick;
        short   saved_frame;
        short   wait;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        head_anim_target_state = 8;
        head_anim_mode         = HEAD_ANIM_DISABLED;
        lcp_wait_head_reach_target();

        saved_frame            = head_sprite_frame;
        head_anim_target_state = HEAD_ANIM_DISABLED;
        head_anim_current      = HEAD_ANIM_DISABLED;

        wave_count = randomRange(20, 40);
        prev_pick  = 0;
        pick       = 0;
        while (wave_count != 0) {
                while (pick == prev_pick)
                        pick = randomRange(0, 2);
                prev_pick = pick;

                if (pick == 0) {
                        head_sprite_frame = 5;
                        play_soundeffect_tv_click();
                } else if (pick == 1) {
                        head_sprite_frame = 6;
                        if (randomRange(0, 1) == 0)
                                play_soundeffect_greeting();
                        else
                                play_soundeffect_speech();
                } else {
                        head_sprite_frame = 4;
                        play_soundeffect_head_nod();
                }
                wait = randomRange(1, 2);
                game_tick_and_animate(wait);
                soundeffect_remaining_ticks = (long) wait;
                wave_count = wave_count - 1;
        }

        head_anim_target_state = 8;
        head_anim_current      = 8;
        head_sprite_frame      = saved_frame;
        game_tick_and_animate(0);
}

/* action_yawn_and_stretch: 15-frame idle yawn.
   addr: action_yawn_and_stretch() */

void
action_yawn_and_stretch()
{
        short   i;

        PLAYER_STATE_ARRAY[0]  = STATE_YAWN_MOUTH_OPEN;
        PLAYER_STATE_ARRAY[1]  = STATE_YAWN_STRETCH_ARMS;
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

/* action_nod_head: 3-frame nod with SFX.
   addr: action_nod_head() */

void
action_nod_head()
{
        short   saved_frame;

        PLAYER_STATE_ARRAY[0]  = STATE_WALK_FRAME_3_STEP;
        PLAYER_STATE_ARRAY[1]  = STATE_WALK_FRAME_4;
        PLAYER_STATE_ARRAY[2]  = STATE_WALK_FRAME_5;
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        head_anim_target_state = 8;
        head_anim_mode         = HEAD_ANIM_DISABLED;
        lcp_wait_head_reach_target();

        saved_frame            = head_sprite_frame;
        head_anim_target_state = HEAD_ANIM_DISABLED;
        head_anim_current      = HEAD_ANIM_DISABLED;

        head_sprite_frame = PLAYER_STATE_ARRAY[0];
        game_tick_and_animate(1);
        head_sprite_frame = PLAYER_STATE_ARRAY[1];
        game_tick_and_animate(1);
        head_sprite_frame = PLAYER_STATE_ARRAY[2];
        game_tick_and_animate(2);

        head_anim_target_state = 8;
        head_anim_current      = 8;
        head_sprite_frame      = saved_frame;
        game_tick_and_animate(0);
}

/* action_pet_dog: call the dog if not already pettable, then wait
   100..200 frames (10 during intro) or until a new event queues.
   addr: action_pet_dog() */

extern void action_call_dog();

void
action_pet_dog()
{
        short   ticks;

        action_interruptible_flag = YES;
        if (dog_pettable_flag == NO)
                action_call_dog();
        action_interruptible_flag = NO;

        ticks = randomRange(100, 200);
        if (intro_sequence_active != NO)
                ticks = 10;

        do {
                ticks = ticks - 1;
                if (ticks == 0)
                        break;
                game_tick_and_animate(0);
        } while (triggered_event_list[0] == ACTION_NONE);

        dog_pettable_flag = NO;
        lcp_state         = STATE_STAND_SIDE_VIEW;
        game_tick_and_animate(0);
}

/* action_call_dog: walk to POS_BTM_DOG_FOOD, crouch, set dog_pettable_flag.
   Real Ghidra behaviour -- see previous session for the derivation.
   addr: action_call_dog() */

void
action_call_dog()
{
        short   result;

        house_get_position_xy(43 /* POS_BTM_DOG_FOOD */,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        lcp_facing_direction   = FACING_RIGHT;
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();
        lcp_state = 35 /* STATE_EXERCISE_CROUCH */;
        game_tick_and_animate(5);
        dog_pettable_flag = YES;
}
