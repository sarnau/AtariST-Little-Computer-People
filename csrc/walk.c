/*
 * walk.c -- LCP & dog pathfinding + step animation.
 *
 * Four functions:
 *   lcp_walk_to_destination()   -- outer loop: steps toward walk_target_
 *                                  x/y until arrival or interruption.
 *   lcp_pathfind_one_step()     -- inner loop: one 8Hz step, handles flat
 *                                  walking, 3 staircase phases (climb,
 *                                  top-of-flight, descend, bottom-of-
 *                                  flight), footstep sound triggers.
 *   lcp_calc_floor_waypoint()   -- when there's no active waypoint,
 *                                  routes the resident to the next
 *                                  staircase entry (or straight to
 *                                  destination if same floor).
 *   dog_calc_walk_path()        -- same waypoint math for the dog.
 *   lcp_play_footstep_sound()   -- selects carpet/wood/stairs SFX
 *                                  based on floor + X position.
 *
 * addr: lcp_walk_to_destination(), lcp_pathfind_one_step(),
 *       lcp_calc_floor_waypoint(), dog_calc_walk_path(),
 *       lcp_play_footstep_sound()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"

extern void     spritedata_select_carried_object_left();
extern void     spritedata_select_carried_object_right();
extern void     sprite_update_slots();
extern void     soundeffect_select();
extern void     lcp_calc_floor_waypoint();
extern void     lcp_pathfind_one_step();
extern void     lcp_play_footstep_sound();

/* lcp_walk_to_destination: pump lcp_pathfind_one_step() until arrival.
   Returns 0 on natural arrival, -1 on interruption (only when the
   resident is idle enough for the queue to preempt).
   addr: lcp_walk_to_destination() */

short
lcp_walk_to_destination()
{
        head_anim_mode       = HEAD_ANIM_WALKING;
        head_anim_state_last = 0;

        do {
                if (walk_target_x == 0 && walk_target_y == 0)
                        return 0;
                lcp_pathfind_one_step();
        } while (in_execute_event_routine_flag != NO ||
                 triggered_event_list[0] == ACTION_NONE ||
                 lcp_carrying_object_flag != NO ||
                 intro_sequence_active != NO ||
                 lcp_on_stairs_flag != NO ||
                 action_interruptible_flag != NO);

        walk_target_y = 0;
        walk_target_x = 0;
        return -1;
}

/* lcp_calc_floor_waypoint: pick the next waypoint given the current
   resident position and the destination.  Same-floor destinations
   route straight to walk_target_x/y; cross-floor destinations route
   through the appropriate entry of staircase_waypoint_coords[] first.
   The middle-floor case has an extra pair-of-flights landing branch
   (stair_top_y_threshold / _bottom_y_threshold) that the top and
   bottom floors don't need.

   addr: lcp_calc_floor_waypoint() */

void
lcp_calc_floor_waypoint()
{
        short   target_floor;
        short   current_floor;
        short   dest_floor;
        short   stair_index;

        target_floor  = get_floor_number_from_y(walk_target_y);
        current_floor = get_floor_number_from_y(lcp_y);

        if (current_floor == target_floor) {
                lcp_on_stairs_flag = NO;
                walk_waypoint_x = walk_target_x;
                walk_waypoint_y = walk_target_y;
                return;
        }

        target_floor    = get_floor_number_from_y(lcp_y);
        stair_index     = (target_floor - 1) + (target_floor - 1);
        current_floor   = stair_index;
        walk_waypoint_x = staircase_waypoint_coords[stair_index];
        walk_waypoint_y = staircase_waypoint_coords[current_floor + 1];

        target_floor = get_floor_number_from_y(lcp_y);
        if (target_floor == 2) {
                target_floor = get_floor_number_from_y(walk_target_y);
                dest_floor   = get_floor_number_from_y(lcp_y);
                if (target_floor < dest_floor) {
                        walk_waypoint_x = stair_top_y_threshold;
                        walk_waypoint_y = stair_bottom_y_threshold;
                }
        }

        lcp_on_stairs_flag = NO;
        if (lcp_x == walk_waypoint_x && lcp_y == walk_waypoint_y) {
                lcp_on_stairs_flag = YES;
                if (walk_target_y < lcp_y) {
                        walk_waypoint_x = staircase_waypoint_coords[current_floor + 2];
                        walk_waypoint_y = staircase_waypoint_coords[current_floor + 3];
                } else {
                        walk_waypoint_y = staircase_waypoint_coords[current_floor - 1];
                        walk_waypoint_x = staircase_waypoint_coords[current_floor - 2];
                }
                target_floor = get_floor_number_from_y(lcp_y);
                if (target_floor == 1) {
                        walk_waypoint_x = stair_top_y_threshold;
                        walk_waypoint_y = stair_bottom_y_threshold;
                }
        }
}

/* dog_calc_walk_path: dog waypoint math.  Structurally identical to
   lcp_calc_floor_waypoint but uses dog_x/y/target/waypoint and applies
   a -3 X patch on the middle-floor landing plus an -8 X kick when the
   dog crests the top of a staircase.
   addr: dog_calc_walk_path() */

void
dog_calc_walk_path()
{
        short   target_floor;
        short   current_floor;
        short   dest_floor;
        short   stair_index;

        target_floor  = get_floor_number_from_y(dog_target_y);
        current_floor = get_floor_number_from_y(dog_y);

        if (current_floor == target_floor) {
                dog_on_stairs_flag = NO;
                dog_waypoint_x = dog_target_x;
                dog_waypoint_y = dog_target_y;
                return;
        }

        target_floor    = get_floor_number_from_y(dog_y);
        stair_index     = (target_floor - 1) + (target_floor - 1);
        current_floor   = stair_index;
        dog_waypoint_x  = staircase_waypoint_coords[stair_index];
        dog_waypoint_y  = staircase_waypoint_coords[current_floor + 1];

        target_floor = get_floor_number_from_y(dog_y);
        if (target_floor == 2) {
                target_floor = get_floor_number_from_y(dog_target_y);
                dest_floor   = get_floor_number_from_y(dog_y);
                if (target_floor < dest_floor) {
                        dog_waypoint_x = stair_top_y_threshold - 3;
                        dog_waypoint_y = stair_bottom_y_threshold;
                }
        }

        dog_on_stairs_flag = NO;
        if (dog_x == dog_waypoint_x && dog_y == dog_waypoint_y) {
                target_floor = get_floor_number_from_y(dog_y);
                if (target_floor == 3)
                        dog_x = dog_x - 8;
                dog_on_stairs_flag = YES;
                if (dog_target_y < dog_y) {
                        dog_waypoint_x = staircase_waypoint_coords[current_floor + 2];
                        dog_waypoint_y = staircase_waypoint_coords[current_floor + 3];
                } else {
                        dog_waypoint_y = staircase_waypoint_coords[current_floor - 1];
                        dog_waypoint_x = staircase_waypoint_coords[current_floor - 2];
                }
                target_floor = get_floor_number_from_y(dog_y);
                if (target_floor == 1) {
                        dog_waypoint_x = stair_top_y_threshold;
                        dog_waypoint_y = stair_bottom_y_threshold;
                }
        }
}

/* lcp_play_footstep_sound: sample-selecting footstep helper.
   footstep_trigger_flag is set by lcp_pathfind_one_step exactly when
   the walk-cycle frame that plants a foot has just landed.
   addr: lcp_play_footstep_sound() */

void
lcp_play_footstep_sound()
{
        short   floor;

        if (footstep_trigger_flag == NO)
                return;

        if (lcp_on_stairs_flag != NO) {
                soundeffect_select(SFX_FOOTSTEP_STAIRS, 2L);
                return;
        }

        floor = get_floor_number_from_y(lcp_y);
        if (floor == 1) {
                if (lcp_x < 166)
                        soundeffect_select(SFX_FOOTSTEP_CARPET, 2L);
                else
                        soundeffect_select(SFX_FOOTSTEP_WOOD, 2L);
        } else if (floor == 2) {
                if (lcp_x > 146 && lcp_x < 234)
                        soundeffect_select(SFX_FOOTSTEP_CARPET, 2L);
        } else if (floor == 3 && lcp_x > 136) {
                soundeffect_select(SFX_FOOTSTEP_WOOD, 2L);
        }
}

/* ---- lcp_pathfind_one_step -------------------------------------------- */

/* Nested-if helper: cycle the walk state through 0..7. */
static void
walk_cycle_state()
{
        if (lcp_state < 8) {
                lcp_state = lcp_state + STATE_WALK_FRAME_1;
                if (lcp_state > 7)
                        lcp_state = STATE_WALK_FRAME_0;
        } else {
                lcp_state = STATE_WALK_FRAME_0;
        }
}

/* Nested-if helper: cycle stair-climb state 9..12 wrapping. */
static void
stair_climb_cycle()
{
        lcp_state = lcp_state + STATE_WALK_FRAME_1;
        if (lcp_state > 12)
                lcp_state = STATE_STAIR_CLIMB_FRAME_0;
}

/* Nested-if helper: set head_anim_target if it isn't already `target`. */
static void
set_head_target(target)
short   target;
{
        if (head_anim_state_last != target) {
                head_anim_target_state = target;
                head_anim_state_last   = target;
        }
}

/* lcp_pathfind_one_step: one 8Hz step along the current waypoint.
   Structurally: (1) if waypoint reached, either destination is reached
   (return) or pick the next waypoint.  (2) If not on stairs, flat walk
   toward waypoint (X first with cycle, then Y in "channel" of the
   floor center).  (3) If on stairs, run the appropriate stair phase
   for the current stair Y bucket.  Footstep flag is set on the two
   step-planting frames per walk cycle.

   addr: lcp_pathfind_one_step() */

void
lcp_pathfind_one_step()
{
        short   floor_num;
        short   x_distance;
        short   next_x;

        footstep_trigger_flag = NO;

        if (walk_target_x == 0 && walk_target_y == 0)
                return;

        if (walk_waypoint_x == 0 && walk_waypoint_y == 0)
                lcp_calc_floor_waypoint();

        /* Exit stair mode when we've reached the target floor. */
        if (lcp_on_stairs_flag != NO) {
                floor_num = get_floor_number_from_y(walk_waypoint_y);
                if (lcp_y <= floor_bottom_y_coords[floor_num - 1]) {
                        if (floor_num == 3)
                                lcp_on_stairs_flag = NO;
                        else if (staircase_waypoint_coords[(floor_num - 1) * 2 + 1] <= lcp_y)
                                lcp_on_stairs_flag = NO;
                }
        }

        /* Waypoint reached? */
        if (lcp_x == walk_waypoint_x && lcp_y == walk_waypoint_y) {
                if (lcp_x == walk_target_x && lcp_y == walk_target_y) {
                        walk_target_x = 0;
                        walk_target_y = 0;
                        lcp_state     = STATE_STAND_IDLE;
                        game_tick_and_animate(0);
                        return;
                }
                lcp_calc_floor_waypoint();
        }

        /* ---- Flat walking (not on stairs) --------------------------- */
        if (lcp_on_stairs_flag == NO) {
                if (lcp_carrying_object_flag != NO)
                        spritedata_select_carried_object_left(lcp_carried_object);

                if (lcp_x < walk_waypoint_x) {
                        lcp_facing_direction = FACING_RIGHT;
                        walk_cycle_state();
                        lcp_x = lcp_x + 1;
                        set_head_target(10);
                } else if (walk_waypoint_x < lcp_x) {
                        lcp_facing_direction = FACING_LEFT;
                        walk_cycle_state();
                        lcp_x = lcp_x - 1;
                        set_head_target(HEAD_ANIM_HORIZONTAL_RANGE |
                                        HEAD_ANIM_SHOWER);
                } else {
                        /* At target X, just cycle animation. */
                        lcp_state = lcp_state + STATE_WALK_FRAME_1;
                        if (lcp_state > 7)
                                lcp_state = STATE_WALK_FRAME_0;
                }

                x_distance = (lcp_x < walk_waypoint_x)
                             ? (walk_waypoint_x - lcp_x)
                             : (lcp_x - walk_waypoint_x);
                if (x_distance < 8) {
                        if (lcp_y < walk_waypoint_y)
                                lcp_y = lcp_y + 1;
                        else if (walk_waypoint_y < lcp_y)
                                lcp_y = lcp_y - 1;
                } else {
                        floor_num = get_floor_number_from_y(lcp_y);
                        if (lcp_y < floor_center_y_coords[floor_num - 1])
                                lcp_y = lcp_y + 1;
                        floor_num = get_floor_number_from_y(lcp_y);
                        if (floor_center_y_coords[floor_num - 1] < lcp_y)
                                lcp_y = lcp_y - 1;
                }

                if (lcp_state == STATE_WALK_FRAME_3_STEP ||
                    lcp_state == STATE_WALK_FRAME_7_STEP)
                        footstep_trigger_flag = YES;
        }

        /* ---- Stair traversal --------------------------------------- */
        if (lcp_on_stairs_flag != NO) {
                if (walk_waypoint_y < lcp_y) {
                        /* Ascending */
                        if (lcp_y == 161) {
                                if (lcp_carrying_object_flag != NO)
                                        spritedata_select_carried_object_left(lcp_carried_object);
                                lcp_state = STATE_STAIR_CLIMB_FRAME_0;
                                lcp_facing_direction = FACING_LEFT;
                                lcp_x = lcp_x - 6;
                                lcp_y = lcp_y - 2;
                                set_head_target(HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER);
                        } else if (lcp_y == 100) {
                                if (lcp_carrying_object_flag != NO)
                                        spritedata_select_carried_object_left(lcp_carried_object);
                                lcp_state = STATE_STAIR_CLIMB_FRAME_0;
                                lcp_facing_direction = FACING_RIGHT;
                                lcp_x = lcp_x + 3;
                                lcp_y = lcp_y - 2;
                                set_head_target(10);
                        } else if (lcp_y < 162 &&
                                   (lcp_y < 101 || lcp_y > 139)) {
                                if (lcp_y < 100) {
                                        /* Upper flight of stairs, going up-right */
                                        if (lcp_carrying_object_flag != NO)
                                                spritedata_select_carried_object_left(lcp_carried_object);
                                        lcp_facing_direction = FACING_RIGHT;
                                        lcp_y = lcp_y - 1;
                                        next_x = lcp_x;
                                        if (lcp_state != STATE_STAIR_CLIMB_FRAME_3_STEP) {
                                                next_x = lcp_x + 1;
                                                if (next_x != walk_waypoint_x)
                                                        next_x = lcp_x + 2;
                                        }
                                        lcp_x = next_x;
                                        stair_climb_cycle();
                                        if (lcp_state == STATE_STAIR_CLIMB_FRAME_3_STEP)
                                                footstep_trigger_flag = YES;
                                        set_head_target(10);
                                } else if (lcp_y < 161) {
                                        /* Lower flight, going up-left */
                                        if (lcp_carrying_object_flag != NO)
                                                spritedata_select_carried_object_left(lcp_carried_object);
                                        lcp_facing_direction = FACING_LEFT;
                                        lcp_y = lcp_y - 1;
                                        next_x = lcp_x;
                                        if (lcp_state != STATE_STAIR_CLIMB_FRAME_3_STEP) {
                                                next_x = lcp_x - 1;
                                                if (next_x != walk_waypoint_x)
                                                        next_x = lcp_x - 2;
                                        }
                                        lcp_x = next_x;
                                        stair_climb_cycle();
                                        if (lcp_state == STATE_STAIR_CLIMB_FRAME_3_STEP)
                                                footstep_trigger_flag = YES;
                                        set_head_target(HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER);
                                }
                        } else {
                                /* Top-of-stair frame (state 13..16). */
                                if (lcp_carrying_object_flag != NO) {
                                        sprite_layer_flags[lcp_carried_object] = SPRITE_BEHIND_LCP;
                                        sprite_update_slots();
                                }
                                if (lcp_state < 13 || lcp_state > 16) {
                                        lcp_state = STATE_STAIR_TOP_FRAME_0;
                                } else {
                                        lcp_state = lcp_state + STATE_WALK_FRAME_1;
                                        if (lcp_state > 16) {
                                                lcp_state = STATE_STAIR_TOP_FRAME_0;
                                                lcp_facing_direction =
                                                        lcp_facing_direction ^ FACING_LEFT;
                                        }
                                        if (lcp_state == STATE_STAIR_TOP_FRAME_3_STEP ||
                                            lcp_state == STATE_STAIR_TOP_FRAME_0)
                                                lcp_y = lcp_y - 2;
                                        if (lcp_state == STATE_STAIR_TOP_FRAME_3_STEP)
                                                footstep_trigger_flag = YES;
                                }
                                set_head_target(HEAD_ANIM_HORIZONTAL_RANGE);
                        }
                } else if (lcp_y < walk_waypoint_y) {
                        /* Descending */
                        if (lcp_carrying_object_flag != NO)
                                spritedata_select_carried_object_left(lcp_carried_object);

                        if (lcp_y == 161) {
                                lcp_state = STATE_STAIR_BTM_FRAME_0;
                                lcp_facing_direction = FACING_RIGHT;
                                lcp_y = 165;
                                lcp_x = lcp_x + 6;
                                set_head_target(8);
                                if (lcp_carrying_object_flag != NO)
                                        spritedata_select_carried_object_right(lcp_carried_object);
                        } else if (lcp_y == 100) {
                                lcp_state = STATE_STAIR_BTM_FRAME_0;
                                lcp_facing_direction = FACING_RIGHT;
                                lcp_y = 102;
                                lcp_x = lcp_x - 2;
                                set_head_target(8);
                                if (lcp_carrying_object_flag != NO)
                                        spritedata_select_carried_object_right(lcp_carried_object);
                        } else if (lcp_y < 162 &&
                                   (lcp_y < 101 || lcp_y > 131)) {
                                if (lcp_y < 100) {
                                        /* Upper flight, going down-left */
                                        if (lcp_carrying_object_flag != NO)
                                                spritedata_select_carried_object_right(lcp_carried_object);
                                        lcp_facing_direction = FACING_LEFT;
                                        lcp_y = lcp_y + 1;
                                        next_x = lcp_x;
                                        if (lcp_state != STATE_STAIR_DESCEND_FRAME_3_STEP) {
                                                next_x = lcp_x - 1;
                                                if (next_x != walk_waypoint_x)
                                                        next_x = lcp_x - 2;
                                        }
                                        lcp_x = next_x;
                                        if (lcp_state < 21 && lcp_state > 16) {
                                                lcp_state = lcp_state + STATE_WALK_FRAME_1;
                                                if (lcp_state > 20)
                                                        lcp_state = STATE_STAIR_DESCEND_FRAME_0;
                                        } else {
                                                lcp_state = STATE_STAIR_DESCEND_FRAME_0;
                                        }
                                        set_head_target(HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER);
                                        if (lcp_state == STATE_STAIR_DESCEND_FRAME_1)
                                                footstep_trigger_flag = YES;
                                } else if (lcp_y < 161) {
                                        /* Lower flight, going down-right */
                                        if (lcp_carrying_object_flag != NO)
                                                spritedata_select_carried_object_right(lcp_carried_object);
                                        lcp_facing_direction = FACING_RIGHT;
                                        lcp_y = lcp_y + 1;
                                        next_x = lcp_x;
                                        if (lcp_state != STATE_STAIR_DESCEND_FRAME_3_STEP) {
                                                next_x = lcp_x + 1;
                                                if (next_x != walk_waypoint_x)
                                                        next_x = lcp_x + 2;
                                        }
                                        lcp_x = next_x;
                                        if (lcp_state < 21 && lcp_state > 16) {
                                                lcp_state = lcp_state + STATE_WALK_FRAME_1;
                                                if (lcp_state > 20)
                                                        lcp_state = STATE_STAIR_DESCEND_FRAME_0;
                                        } else {
                                                lcp_state = STATE_STAIR_DESCEND_FRAME_0;
                                        }
                                        set_head_target(10);
                                        if (lcp_state == STATE_STAIR_DESCEND_FRAME_1)
                                                footstep_trigger_flag = YES;
                                }
                        } else {
                                /* Bottom-of-stair frame (state 21..24). */
                                if (lcp_carrying_object_flag != NO)
                                        spritedata_select_carried_object_right(lcp_carried_object);
                                if (lcp_state < 21 || lcp_state > 24) {
                                        lcp_state = STATE_STAIR_BTM_FRAME_0;
                                        lcp_x = lcp_x + 2;
                                } else {
                                        lcp_state = lcp_state + STATE_WALK_FRAME_1;
                                        if (lcp_state > 24) {
                                                lcp_state = STATE_STAIR_BTM_FRAME_0;
                                                lcp_facing_direction =
                                                        lcp_facing_direction ^ FACING_LEFT;
                                        }
                                        if (lcp_state == STATE_STAIR_BTM_FRAME_1 ||
                                            lcp_state == STATE_STAIR_BTM_FRAME_2)
                                                lcp_y = lcp_y + 2;
                                        if (lcp_state == STATE_STAIR_BTM_FRAME_3)
                                                footstep_trigger_flag = YES;
                                }
                                set_head_target(8);
                        }
                }
        }

        /* Sickness slows the walk: two ticks per step and delayed
           footstep sound.  Healthy: one tick with immediate sound. */
        if (lcp.sickness_level != SICKNESS_HEALTHY) {
                game_tick_and_animate(0);
                lcp_play_footstep_sound();
        }
        game_tick_and_animate(0);
        if (lcp.sickness_level == SICKNESS_HEALTHY)
                lcp_play_footstep_sound();
}
