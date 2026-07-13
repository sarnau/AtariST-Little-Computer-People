/*
 * actions_food.c -- meal, kitchen, feed-dog, snack handlers.
 *
 * All four share the kitchen-cabinet / fridge / stove workflow and
 * update food-supply / hunger / dog-bowl state at their tail.
 *
 * addr: action_eat_meal(), action_kitchen_cabinet(),
 *       action_feed_dog(), action_get_snack_from_fridge()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     spritedata_select_carried_object_left();
extern void     spritedata_select_carried_object_right();
extern void     spritedata_select();
extern void     sprite_update_slots();
extern void     soundeffect_select();
extern void     object_draw();
extern void     action_open_close_cabinet();
extern void     action_open_close_fridge();
extern void     action_kitchen_cabinet();
extern void     screen_draw_food_cabinet();
extern void     lcp_check_recovery();

/* action_eat_meal: pot from cabinet -> stove (with cooking animation)
   -> table setting; ends with a kitchen_cabinet call to actually eat.
   addr: action_eat_meal() */

void
action_eat_meal()
{
        short   result;
        short   counter;
        short   pick;

        house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
        lcp_state = STATE_REACH_FORWARD;game_tick_and_animate(2);
        lcp_state = STATE_STAND_FACING_SCREEN; game_tick_and_animate(0);

        /* Pot from cabinet to stove */
        spritedata_select_carried_object_left(SPRITE_COOKING_POT);
        house_get_position_xy(POS_BTM_STOVE,
                              &walk_target_x, &walk_target_y);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();

        sprite_layer_flags[SPRITE_COOKING_POT] = SPRITE_HIDDEN;
        sprite_update_slots();
        spritedata_select_carried_object_left(SPRITE_COOKING_POT);
        lcp_carrying_object_flag = NO;
        sprite_pending_x[sprite_slot_map[SPRITE_COOKING_POT]] = 11;
        sprite_pending_y[sprite_slot_map[SPRITE_COOKING_POT]] = 172;

        lcp_facing_direction = FACING_LEFT;
        lcp_state            = STATE_BEND_AND_REACH;

        /* 30..50 tick cooking animation, rotating stove frames. */
        counter = randomRange(30, 50);
        while (counter != 0) {
                pick = randomRange(0, 2);
                object_draw(object_id_stove_animation[pick], 6, 172);
                game_tick_and_animate(1);
                counter = counter - 1;
        }
        object_draw(object_id_stove_off, 6, 172);

        sprite_layer_flags[SPRITE_COOKING_POT] = SPRITE_HIDDEN;
        sprite_update_slots();
        spritedata_select_carried_object_left(SPRITE_55);

        /* Back to cabinet, then chain into kitchen_cabinet to eat. */
        house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                              &walk_target_x, &walk_target_y);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();
        sprite_layer_flags[SPRITE_55] = SPRITE_HIDDEN;
        sprite_update_slots();
        lcp_carrying_object_flag = NO;
        game_tick_and_animate(0);
        action_kitchen_cabinet();
        action_interruptible_flag = NO;
}

/* action_kitchen_cabinet: the eat routine.  Open cabinet, decrement
   food count, carry package to table, eat 10..20 bite/chew cycles,
   return the package.  This is where hunger actually gets reset.
   addr: action_kitchen_cabinet() */

void
action_kitchen_cabinet()
{
        short           saved_head_frame;
        short           chew_delay;
        short           eat_cycles;
        short           inner;
        unsigned short  food_count;
        short           roll;

        PLAYER_STATE_ARRAY[0] = STATE_EAT_BITE;
        PLAYER_STATE_ARRAY[1] = STATE_EAT_CHEW;
        action_interruptible_flag = YES;

        house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        action_open_close_cabinet(0);

        food_count = (lcp.door_states_and_flags >> 9) & 7;
        if (food_count == 0) {
                game_tick_and_animate(2);
                action_interruptible_flag = NO;
                return;
        }

        /* Take one package: decrement the 3-bit food-count nibble. */
        lcp_state = STATE_REACH_INTO_CABINET;
        game_tick_and_animate(3);
        lcp.door_states_and_flags =
                (lcp.door_states_and_flags & ~DSF_FOOD_MASK) |
                ((food_count - 1) * 0x200);
        screen_draw_food_cabinet();
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(2);

        roll = randomRange(0, 100);
        if (lcp.initiative_threshold < roll)
                action_open_close_cabinet(1);

        spritedata_select_carried_object_left(SPRITE_FOOD_PACKAGE);
        house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();
        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();

        /* Drop a table setting sprite in the foreground. */
        sprite_layer_flags[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        spritedata_select(SPRITE_TABLE_SETTING);
        sprite_pending_x[sprite_slot_map[SPRITE_TABLE_SETTING]] = 103;
        sprite_pending_y[sprite_slot_map[SPRITE_TABLE_SETTING]] = 180;

        house_get_position_xy(POS_BTM_TABLE_RIGHT,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();
        house_get_position_xy(POS_BTM_TABLE_LEFT,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();

        head_anim_mode       = HEAD_ANIM_DISABLED;
        lcp_state            = STATE_STAND_SIDE_VIEW;
        lcp_facing_direction = FACING_RIGHT;
        spritedata_select_carried_object_right(SPRITE_FOOD_PACKAGE);
        head_anim_target_state = 8;
        lcp_wait_head_reach_target();

        saved_head_frame = head_sprite_frame;
        lcp_state        = PLAYER_STATE_ARRAY[0];
        lcp_y = lcp_y + 8;
        lcp_x = lcp_x + 6;
        eat_cycles       = randomRange(10, 20);
        head_anim_target_state = HEAD_ANIM_DISABLED;
        head_anim_current      = HEAD_ANIM_DISABLED;
        game_tick_and_animate(0);
        lcp_carrying_object_flag = NO;
        sprite_pending_x[sprite_slot_map[SPRITE_FOOD_PACKAGE]] =
                sprite_pending_x[sprite_slot_map[SPRITE_FOOD_PACKAGE]] + 3;
        sprite_pending_y[sprite_slot_map[SPRITE_FOOD_PACKAGE]] =
                sprite_pending_y[sprite_slot_map[SPRITE_FOOD_PACKAGE]] - 4;
        game_tick_and_animate(0);

        while (eat_cycles > 0) {
                lcp_state = PLAYER_STATE_ARRAY[1];
                game_tick_and_animate(2);
                head_sprite_frame = 0;
                chew_delay = randomRange(1, 2);
                game_tick_and_animate(chew_delay);
                lcp_state = PLAYER_STATE_ARRAY[0];
                head_sprite_frame = saved_head_frame;
                game_tick_and_animate(0);

                inner = randomRange(4, 8);
                while (inner > 0 &&
                       triggered_event_list[0] == ACTION_NONE) {
                        chew_delay = randomRange(1, 2);
                        game_tick_and_animate(chew_delay);
                        head_sprite_frame = 1;
                        game_tick_and_animate(0);
                        head_sprite_frame = 2;
                        game_tick_and_animate(0);
                        inner = inner - 1;
                }
                head_sprite_frame = saved_head_frame;
                eat_cycles = eat_cycles - 1;
        }

        lcp_carrying_object_flag = YES;
        head_anim_target_state   = 8;
        head_anim_current        = 8;
        spritedata_select_carried_object_left(SPRITE_FOOD_PACKAGE);
        lcp_y = lcp_y - 8;
        lcp_x = lcp_x - 6;
        lcp_state = STATE_STAND_SIDE_VIEW;
        lcp_wait_head_reach_target();
        game_tick_and_animate(0);

        house_get_position_xy(POS_BTM_TABLE_RIGHT,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();
        house_get_position_xy(POS_BTM_KITCHEN_SINK,
                              &walk_target_x, &walk_target_y);
        lcp_walk_to_destination();

        sprite_layer_flags[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        sprite_update_slots();
        sprite_layer_flags[SPRITE_FOOD_PACKAGE]  = SPRITE_HIDDEN;
        sprite_update_slots();
        lcp_carrying_object_flag = NO;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        game_tick_and_animate(4);

        lcp.hunger_level   = NEED_SATISFIED;
        lcp.bathroom_timer = lcp.bathroom_timer_max;
        lcp_check_recovery();
        action_interruptible_flag = NO;
}

/* action_feed_dog: fridge -> dog bowl -> fridge.  Called both
   standalone (value == 0, open fridge first) and from the Ctrl+D
   delivery path (value == 1, already have the package in hand).
   addr: action_feed_dog() */

void
action_feed_dog(value)
short   value;
{
        short   result;

        if (value == 0) {
                house_get_position_xy(POS_BTM_FRIDGE,
                                      &walk_target_x, &walk_target_y);
                result = lcp_walk_to_destination();
                if (result != 0)
                        return;

                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();

                lcp_facing_direction = FACING_LEFT;
                lcp_state            = STATE_REACH_INTO_CABINET;
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
                game_tick_and_animate(2);

                object_draw(object_id_fridge_open_1, 24, 153);
                game_tick_and_animate(1);
                object_draw(object_id_fridge_closed, 24, 153);
                soundeffect_select(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(1);

                spritedata_select_carried_object_left(SPRITE_FOOD_PACKAGE);
        }

        /* Package -> dog bowl (fill it). */
        house_get_position_xy(POS_BTM_DOG_BOWL,
                              &walk_target_x, &walk_target_y);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        head_anim_target_state = HEAD_ANIM_HORIZONTAL_RANGE;
        sprite_layer_flags[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
        sprite_update_slots();
        lcp_carrying_object_flag = NO;
        lcp_wait_head_reach_target();

        lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);
        lcp_state = STATE_REACH_FORWARD;game_tick_and_animate(2);
        lcp_state = STATE_BEND_DOWN;    game_tick_and_animate(1);

        dog_food_bowl_change = 1;
        lcp_dog_bowl_status  = BOWL_FULL;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);

        /* Package back to fridge. */
        spritedata_select_carried_object_left(SPRITE_FOOD_PACKAGE);
        house_get_position_xy(POS_BTM_FRIDGE,
                              &walk_target_x, &walk_target_y);
        action_interruptible_flag = YES;
        lcp_walk_to_destination();

        sprite_layer_flags[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
        sprite_update_slots();
        lcp_carrying_object_flag = NO;
        action_open_close_fridge();
        action_interruptible_flag = NO;
}

/* action_get_snack_from_fridge: trampoline into action_open_close_fridge
   after walking to the fridge.
   addr: action_get_snack_from_fridge() */

void
action_get_snack_from_fridge()
{
        short   result;

        house_get_position_xy(POS_BTM_FRIDGE,
                              &walk_target_x, &walk_target_y);
        result = lcp_walk_to_destination();
        if (result == 0)
                action_open_close_fridge();
}
