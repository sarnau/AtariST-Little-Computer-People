/*
 * actions_house.c -- walk-and-interact action handlers.
 *
 * Ports for actions that walk somewhere in the house, play an
 * interaction animation with SFX, and update world state.
 *
 * addr: action_read_newspaper(), action_get_in_out_of_bed(),
 *       action_dance(), action_drink(), action_use_toilet(),
 *       action_wake_up_morning(), action_go_to_bed_night()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include <osbind.h>             /* Random() */

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     spritedata_select_carried_object_left();
extern void     spritedata_select();
extern void     sprite_update_slots();
extern void     soundeffect_select();
extern void     object_draw();
extern void     tv_turn_on();
extern void     tv_turn_off();
extern void     update_water_level_bar();
extern void     lcp_check_recovery();
extern void     action_drink_water_animation();
extern void     action_listen_song();
extern void     action_wake_from_alarm();
extern void     action_take_shower();
extern void     action_brush_teeth();
extern void     action_open_close_bedroom_closet();
extern void     action_eat_meal();
extern void     action_close_toilet_door();
extern void     action_kitchen_cabinet();
extern void     hide_lcp_sprites();
extern void     show_lcp_sprites();

/* action_read_newspaper: armchair + TV + 200-frame reading loop.
   addr: action_read_newspaper() */

void
action_read_newspaper()
{
        short           result;
        unsigned short  rnd;
        short           t;

        PLAYER_STATE_ARRAY[0] = STATE_READ_PAPER_HOLD;
        PLAYER_STATE_ARRAY[1] = STATE_READ_PAPER_TURN_PAGE;
        tv_turn_on();
        house_get_position_xy(POS_TOP_ARMCHAIR,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        head_anim_mode         = HEAD_ANIM_READING;
        lcp_facing_direction   = FACING_LEFT;
        lcp_state              = STATE_SIT_IN_ARMCHAIR;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER;
        lcp_wait_head_reach_target();
        lcp_y = lcp_y + 8;

        t = 0;
        while (t < 200 && triggered_event_list[0] == ACTION_NONE) {
                lcp_facing_direction = FACING_LEFT;
                lcp_state            = PLAYER_STATE_ARRAY[0];
                rnd = (unsigned short) Random();
                if ((rnd & 0xf) == 5)
                        lcp_state = PLAYER_STATE_ARRAY[1];
                game_tick_and_animate(1);
                t = t + 1;
        }

        lcp_y = lcp_y - 8;
        lcp_facing_direction = FACING_LEFT;
        lcp_state = STATE_SIT_IN_ARMCHAIR;
        game_tick_and_animate(2);
        tv_turn_off();
}

/* action_get_in_out_of_bed: undress and lie down, or reverse.
   addr: action_get_in_out_of_bed() */

void
action_get_in_out_of_bed()
{
        short   result;

        PLAYER_STATE_ARRAY[0] = STATE_UNDRESS_AT_BED;
        PLAYER_STATE_ARRAY[1] = STATE_LIE_DOWN_GETTING_IN;
        PLAYER_STATE_ARRAY[2] = STATE_LIE_DOWN_IN_BED;

        if (lcp.is_sleeping == NO) {
                house_get_position_xy(POS_MID_BED,
                                      &walk_target_x, &walk_target_y);
                result = lcp_walk_to_destination();
                if (result != 0)
                        return;
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_IDLE;
                head_anim_target_state = 10;
                lcp_wait_head_reach_target();
                lcp.is_sleeping = YES;
                lcp_x = lcp_x - 10;
                lcp_facing_direction = FACING_RIGHT;
                lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(2);
                lcp_x = lcp_x - 8;
                lcp_state = PLAYER_STATE_ARRAY[1]; game_tick_and_animate(2);
                lcp_x = lcp_x - 2;
                lcp_state = PLAYER_STATE_ARRAY[2]; game_tick_and_animate(2);
        } else {
                lcp_facing_direction = FACING_RIGHT;
                lcp_x = lcp_x + 10;
                lcp_state = STATE_LIE_DOWN_GETTING_IN; game_tick_and_animate(2);
                lcp_x = lcp_x + 10;
                lcp_state = PLAYER_STATE_ARRAY[0]; game_tick_and_animate(2);
                lcp.is_sleeping = NO;
                lcp_state              = STATE_STAND_IDLE;
                head_anim_target_state = 10;
                lcp_wait_head_reach_target();
                game_tick_and_animate(2);
        }
}

/* action_dance: turn on the record player if needed, then step-shift
   until the song ends or the event queue interrupts.
   addr: action_dance() */

void
action_dance()
{
        short   result;
        short   i;

        PLAYER_STATE_ARRAY[0] = STATE_DANCE_STEP_LEFT;
        PLAYER_STATE_ARRAY[1] = STATE_DANCE_STEP_RIGHT;

        if (lcp_record_playing == NO) {
                action_interruptible_flag = YES;
                action_listen_song();
        }
        action_interruptible_flag = NO;

        house_get_position_xy(POS_TOP_DANCE_FLOOR,
                              &walk_target_x, &walk_target_y);
        walk_target_y = walk_target_y + 8;
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();

        i = 0;
        while (midi_is_playing != NO) {
                i = i + 1;
                lcp_state = PLAYER_STATE_ARRAY[i & 1];
                if (triggered_event_list[0] != ACTION_NONE)
                        break;
                game_tick_and_animate(2);
        }

        lcp_state = STATE_STAND_SIDE_VIEW;
        game_tick_and_animate(0);
}

/* action_drink: sink -> glass -> tap -> drink -> reset thirst.
   addr: action_drink() */

void
action_drink()
{
        short   result;

        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        action_interruptible_flag = YES;
        spritedata_select_carried_object_left(SPRITE_GLASS);
        house_get_position_xy(POS_BTM_WATER_TAP,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();

        sprite_layer_flags[SPRITE_GLASS] = SPRITE_HIDDEN;
        sprite_update_slots();
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        if (lcp_water_level != 0) {
                lcp_state = STATE_BEND_DOWN;
                lcp_facing_direction = FACING_RIGHT;
                game_tick_and_animate(0);
                update_water_level_bar(-3);
                head_anim_mode = HEAD_ANIM_DISABLED;
                lcp_state = STATE_DRINK_FROM_GLASS;
                game_tick_and_animate(16);
                lcp_state = STATE_STAND_FACING_SCREEN;
                lcp_y = lcp_y + 1;
                game_tick_and_animate(3);
                action_drink_water_animation(3);
        }

        lcp.thirst_level = NEED_SATISFIED;
        lcp.thirst_timer = lcp.thirst_timer_max;
        lcp_check_recovery();
        sprite_layer_flags[SPRITE_GLASS] = SPRITE_HIDDEN;
        sprite_update_slots();
        lcp_carrying_object_flag = NO;
        action_interruptible_flag = NO;
}

/* action_use_toilet: 3-sprite door animation, sit + flush + refill.
   addr: action_use_toilet() */

void
action_use_toilet()
{
        short   result;
        short   saved_x;
        short   counter;

        house_get_position_xy(POS_MID_TOILET_DOOR,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        /* Open the door if it isn't already. */
        if (lcp_toilet_door_open == NO) {
                lcp_facing_direction = FACING_LEFT;
                lcp_state = STATE_BEND_AND_REACH;
                game_tick_and_animate(2);
                object_draw(object_id_door_toilet_closed, 187, 87);
                game_tick_and_animate(2);
                object_draw(object_id_door_toilet_open_1, 187, 87);
                soundeffect_select(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(2);
                object_draw(object_id_door_toilet_open_2, 187, 87);
                game_tick_and_animate(2);
                lcp_toilet_door_open = YES;
        }

        /* Walk into the toilet cubicle. */
        lcp_facing_direction = FACING_RIGHT;
        sprite_layer_flags[SPRITE_DOOR_ANIM_3] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_DOOR_ANIM_3);
        sprite_pending_x[sprite_slot_map[SPRITE_DOOR_ANIM_3]] = 187;
        sprite_pending_y[sprite_slot_map[SPRITE_DOOR_ANIM_3]] = 87;

        house_get_position_xy(POS_MID_TOILET_DOOR,
                              &walk_target_x, &walk_target_y);
        walk_target_y = walk_target_y - 3;
        walk_target_x = walk_target_x - 10;
        action_interruptible_flag = YES;
        lcp_walk_to_destination();
        saved_x = lcp_x;

        /* Close door behind the resident (3 sprite phases). */
        sprite_layer_flags[SPRITE_DOOR_ANIM_3] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_DOOR_ANIM_2] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_DOOR_ANIM_2);
        sprite_pending_x[sprite_slot_map[SPRITE_DOOR_ANIM_2]] = 187;
        sprite_pending_y[sprite_slot_map[SPRITE_DOOR_ANIM_2]] = 87;
        object_draw(object_id_door_toilet_open_1, 187, 87);
        game_tick_and_animate(1);

        sprite_layer_flags[SPRITE_DOOR_ANIM_2] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_DOOR_ANIM_1] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_DOOR_ANIM_1);
        sprite_pending_x[sprite_slot_map[SPRITE_DOOR_ANIM_1]] = 187;
        sprite_pending_y[sprite_slot_map[SPRITE_DOOR_ANIM_1]] = 87;
        object_draw(object_id_door_toilet_closed, 187, 87);
        hide_lcp_sprites();
        soundeffect_select(SFX_DOOR_CLOSE, 6L);
        game_tick_and_animate(1);

        /* Do the thing.  45..60 ticks, then flush + 16 tick refill. */
        counter = randomRange(45, 60);
        game_tick_and_animate(counter);
        soundeffect_select(SFX_TOILET_FLUSH, 6L);
        game_tick_and_animate(16);

        /* Reopen door + walk out. */
        sprite_layer_flags[SPRITE_DOOR_ANIM_1] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_DOOR_ANIM_2] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_DOOR_ANIM_2);
        show_lcp_sprites();
        sprite_pending_x[sprite_slot_map[SPRITE_DOOR_ANIM_2]] = 187;
        sprite_pending_y[sprite_slot_map[SPRITE_DOOR_ANIM_2]] = 87;
        object_draw(object_id_door_toilet_open_1, 187, 87);
        soundeffect_select(SFX_DOOR_OPEN, 6L);
        game_tick_and_animate(1);

        sprite_layer_flags[SPRITE_DOOR_ANIM_2] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_DOOR_ANIM_3] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_DOOR_ANIM_3);
        sprite_pending_x[sprite_slot_map[SPRITE_DOOR_ANIM_3]] = 187;
        sprite_pending_y[sprite_slot_map[SPRITE_DOOR_ANIM_3]] = 87;
        object_draw(object_id_door_toilet_open_2, 187, 87);
        game_tick_and_animate(1);
        lcp_toilet_door_open = YES;

        lcp_x = saved_x;
        house_get_position_xy(POS_MID_TOILET_DOOR,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();

        if (lcp_toilet_door_open != NO) {
                sprite_layer_flags[SPRITE_DOOR_ANIM_3] = SPRITE_HIDDEN;
                sprite_update_slots();
                game_tick_and_animate(0);
        }

        counter = randomRange(0, 100);
        if (lcp.initiative_threshold < counter ||
            intro_sequence_active != NO)
                action_close_toilet_door();

        lcp.bathroom_need  = NO;
        lcp.bathroom_timer = 9999;
        action_interruptible_flag = NO;
}

/* action_wake_up_morning: scheduled morning routine.
   addr: action_wake_up_morning() */

void
action_wake_up_morning()
{
        short   counter;

        action_interruptible_flag = YES;
        ctrl_a_alarm_pressed_flag = YES;
        counter = randomRange(40, 100);
        game_tick_and_animate(counter);
        if (lcp.is_sleeping == YES)
                action_get_in_out_of_bed();

        action_interruptible_flag = YES; action_wake_from_alarm();
        action_interruptible_flag = YES; action_take_shower();
        action_interruptible_flag = YES; action_brush_teeth();
        action_interruptible_flag = YES; action_open_close_bedroom_closet(0);
        action_interruptible_flag = YES; action_eat_meal();
        action_interruptible_flag = NO;
}

/* action_go_to_bed_night: scheduled bedtime routine.
   addr: action_go_to_bed_night() */

void
action_go_to_bed_night()
{
        action_interruptible_flag = YES; action_take_shower();
        action_interruptible_flag = YES; action_open_close_bedroom_closet(1);
        action_interruptible_flag = YES; action_kitchen_cabinet();
        action_interruptible_flag = YES; action_brush_teeth();
        action_interruptible_flag = YES; action_get_in_out_of_bed();
        action_interruptible_flag = NO;
}

/* action_get_dressed: pure head-anim routine.  Turns the head to face
   a canonical resting direction, then oscillates the vertical tilt bit
   four times (undressing / dressing motion communicated via head bob).
   No walking, no world state change.
   addr: action_get_dressed() */

void
action_get_dressed()
{
        short   entry_current;
        short   h;
        short   i;

        entry_current = head_anim_current;
        h = head_anim_current & 7;

        if (h == 0 || h == 1 || h == 7)
                head_anim_target_state = 8;
        else if (h == 2)                        /* HEAD_ANIM_SHOWER value */
                head_anim_target_state = 9;
        else if (h == 6)
                head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE |
                                         7 /* HEAD_MODE_H_AMPLITUDE mask */;
        else if (h == 3 || h == 4)
                head_anim_target_state = 10;
        else if (h == 5)
                head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE |
                                         HEAD_ANIM_SHOWER;

        head_anim_mode = HEAD_ANIM_DISABLED;
        lcp_wait_head_reach_target();

        for (i = 0; i < 4; i = i + 1) {
                head_anim_target_state = head_anim_current & 7;
                lcp_wait_head_reach_target();
                head_anim_target_state = head_anim_current | 0x10;
                lcp_wait_head_reach_target();
        }

        head_anim_target_state = entry_current;
        lcp_wait_head_reach_target();
}

/* lcp_idle_look_left / lcp_idle_look_right: the two 4-tick "stand-and-
   look" gestures used by the TV toggle, record player, and post-action
   idle transitions.  The 1985 code sets FACING_RIGHT in both -- the
   "left" / "right" naming refers to which head-frame direction the
   animation actually plays via head_anim_target_state, not the body
   facing.  Preserved verbatim.
   addr: lcp_idle_look_left(), lcp_idle_look_right() */

void
lcp_idle_look_left()
{
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        lcp_state = STATE_BEND_DOWN;
        game_tick_and_animate(4);
        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}

void
lcp_idle_look_right()
{
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        lcp_state = STATE_BEND_DOWN;
        game_tick_and_animate(4);
        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
}
