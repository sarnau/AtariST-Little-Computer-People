/*
 * actions_bathroom.c -- hygiene handlers.
 *
 * All three share the bathroom-sink / shower-cubicle walk-and-animate
 * pattern, without persistent world-state updates (unlike toilet or
 * kitchen).  Water-running SFX is toggled on entry and stopped on exit.
 *
 * addr: action_take_shower(), action_brush_teeth(), action_wash_hands()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>             /* Random() */

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     spritedata_select();
extern void     sprite_update_slots();
extern void     soundeffect_select();
extern void     soundeffects_off();
extern void     spritedata_select_carried_object_left();

/* action_take_shower: enter the shower cubicle, randomly alternate
   scrub / wash blocks for 20..25 cycles, exit.  Head-anim mode gets a
   dedicated HEAD_ANIM_SHOWER so the head bobs left/right in step.
   addr: action_take_shower() */

void
action_take_shower()
{
        short   result;
        short   count;
        short   pick;

        house_get_position_xy(POS_MID_SHOWER_DOOR,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        house_get_position_xy(POS_MID_SHOWER_INSIDE,
                              &walk_target_x, &walk_target_y);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_SHOWER_STAND;
        lcp_x = lcp_x - 8;
        lcp_y = lcp_y - 23;
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();
        head_anim_mode = HEAD_ANIM_SHOWER;

        count = randomRange(20, 25);
        while (count != 0) {
                pick = randomRange(0, 1);
                if (pick == 0) {
                        lcp_state = STATE_SHOWER_SCRUB_LEFT;  game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_SCRUB_RIGHT; game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_SCRUB_LEFT;  game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_SCRUB_RIGHT; game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_STAND;       game_tick_and_animate(4);
                } else {
                        lcp_state = STATE_SHOWER_WASH_LEFT;   game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_WASH_RIGHT;  game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_WASH_LEFT;   game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_WASH_RIGHT;  game_tick_and_animate(2);
                        lcp_state = STATE_SHOWER_STAND;       game_tick_and_animate(4);
                }
                count = count - 1;
        }

        lcp_state = STATE_STAND_FACING_SCREEN;
        lcp_y = lcp_y + 29;
        game_tick_and_animate(2);
        house_get_position_xy(POS_MID_SHOWER_DOOR,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();
        head_anim_mode = HEAD_ANIM_DISABLED;
        action_interruptible_flag = NO;
}

/* action_brush_teeth: 24..35 cycle brush loop.  The "toothbrush"
   sprite is actually SPRITE_STUDY_DOOR_FRAME (id 6) repositioned above
   the resident's head, alternating between two X positions.
   addr: action_brush_teeth() */

void
action_brush_teeth()
{
        unsigned short  brush_cycles;
        short           result;
        short           x_left;
        short           x_right;

        brush_cycles = (unsigned short) randomRange(24, 35);
        house_get_position_xy(POS_MID_BATHROOM_SINK,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        head_anim_mode = HEAD_ANIM_DISABLED;
        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_BRUSH_TEETH;
        head_anim_target_state = 10;
        lcp_y = lcp_y - 2;
        lcp_wait_head_reach_target();

        sprite_layer_flags[SPRITE_STUDY_DOOR_FRAME] = SPRITE_BEHIND_LCP;
        spritedata_select(SPRITE_STUDY_DOOR_FRAME);
        x_left  = lcp_x + 8;
        x_right = lcp_x + 12;
        sprite_pending_x[sprite_slot_map[SPRITE_STUDY_DOOR_FRAME]] = x_left;
        sprite_pending_y[sprite_slot_map[SPRITE_STUDY_DOOR_FRAME]] = lcp_y - 24;

        while (brush_cycles != 0) {
                if (((brush_cycles - 1) & 1) == 0)
                        sprite_pending_x[sprite_slot_map[SPRITE_STUDY_DOOR_FRAME]] = x_right;
                else
                        sprite_pending_x[sprite_slot_map[SPRITE_STUDY_DOOR_FRAME]] = x_left;
                game_tick_and_animate(0);
                brush_cycles = brush_cycles - 1;
        }

        sprite_layer_flags[SPRITE_STUDY_DOOR_FRAME] = SPRITE_HIDDEN;
        sprite_update_slots();
        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        lcp_y = lcp_y + 2;
        game_tick_and_animate(0);
}

/* action_wash_hands: sink + water + 4..127 random wash cycles picking
   from 3 hand-position states.  Stops water on any interruption.
   addr: action_wash_hands() */

void
action_wash_hands()
{
        short           result;
        unsigned short  rnd;
        unsigned short  val;
        unsigned short  last_pick;
        short           counter;

        PLAYER_STATE_ARRAY[0] = STATE_WASH_HANDS_CENTER;
        PLAYER_STATE_ARRAY[1] = STATE_WASH_HANDS_LEFT;
        PLAYER_STATE_ARRAY[2] = STATE_WASH_HANDS_RIGHT;

        house_get_position_xy(POS_MID_BATHROOM_SINK,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        rnd = (unsigned short) Random();
        soundeffect_select(SFX_WATER_RUNNING, 10000L);

        counter   = 0;
        last_pick = 0;
        while (counter < (short) ((rnd & 0x7f) | 4) &&
               triggered_event_list[0] == ACTION_NONE) {
                val = (unsigned short) Random();
                while ((val & 3) == last_pick)
                        val = (unsigned short) Random();
                val = val & 3;
                last_pick = val;
                if (val == 3)
                        lcp_state = PLAYER_STATE_ARRAY[1];
                else
                        lcp_state = PLAYER_STATE_ARRAY[val];
                lcp_facing_direction = (val == 3) ? FACING_LEFT : FACING_RIGHT;
                game_tick_and_animate(1);
                counter = counter + 1;
        }

        if (soundeffect_playing_flag != NO &&
            soundeffect_playing_id == SFX_WATER_RUNNING)
                soundeffects_off();

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

/* action_drink_water_animation: fill / drink a glass (carried_object
   pre-selected by the caller).  Runs the same 3-position hand-shift
   loop as action_wash_hands but scoped to lower amplitudes (bit 0x1f
   instead of 0x7f), so it plays for ~4..35 ticks instead of ~4..127.
   The `value` argument is the SPRITE_ID of the object being carried
   (typically SPRITE_GLASS).
   addr: action_drink_water_animation() */

void
action_drink_water_animation(value)
short   value;
{
        unsigned short  rnd;
        unsigned short  pick;
        unsigned short  last_pick;
        short           counter;

        PLAYER_STATE_ARRAY[0] = STATE_WASH_HANDS_CENTER;
        PLAYER_STATE_ARRAY[1] = STATE_WASH_HANDS_LEFT;
        PLAYER_STATE_ARRAY[2] = STATE_WASH_HANDS_RIGHT;

        spritedata_select_carried_object_left(value);
        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();
        sprite_layer_flags[value] = SPRITE_HIDDEN;
        sprite_update_slots();

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        rnd = (unsigned short) Random();
        soundeffect_select(SFX_WATER_RUNNING, 10000L);

        last_pick = 0;
        for (counter = 0;
             counter < (short) ((rnd & 0x1f) | 4);
             counter = counter + 1) {
                pick = (unsigned short) Random();
                while ((pick & 3) == last_pick)
                        pick = (unsigned short) Random();
                pick = pick & 3;
                last_pick = pick;
                if (pick == 3)
                        lcp_state = PLAYER_STATE_ARRAY[1];
                else
                        lcp_state = PLAYER_STATE_ARRAY[pick];
                lcp_facing_direction = (pick == 3) ? FACING_LEFT : FACING_RIGHT;
                game_tick_and_animate(1);
        }

        if (soundeffect_playing_flag != NO &&
            soundeffect_playing_id == SFX_WATER_RUNNING)
                soundeffects_off();

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}
