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
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern PLAYER   lcp;                            /* the resident LCP */
extern BOOL16   intro_sequence_active;
extern short    triggered_event_list[];
extern BOOL16   in_execute_event_routine_flag;
extern short    lcp_x;
extern short    lcp_y;
extern short    g_hatas;
extern short    g_hamod;
extern BOOL16   action_interruptible_flag;
extern short    g_wtx;
extern short    g_wty;
extern void     game_tick_and_animate();
extern short    g_wyx;
extern short    g_wyy;
extern short    lcp_on_stairs_flag;
extern BOOL16   footstep_trigger_flag;
extern short    g_hastl;
extern short    stair_top_y_threshold;
extern short    stair_bottom_y_threshold;
extern short    get_floor_number_from_y();
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    g_lcyof;
extern short    g_lcieo;
extern short    dog_x;
extern short    dog_y;
extern short    g_dtx;
extern short    g_dty;
extern short    g_dyx;
extern short    g_dyy;
extern short    dog_on_stairs_flag;
extern short    g_selaf[];
extern short    floor_bottom_y_coords[];
extern short    floor_center_y_coords[];
extern short    staircase_waypoint_coords[];
extern void     sp_ssco();
extern void     sp_ss02();
extern void     sp_upds();
extern void     sf_sele();
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
        g_hamod       = HEAD_ANIM_WALKING;
        g_hastl = 0;

        do {
                if (g_wtx == 0 && g_wty == 0)
                        return 0;
                lcp_pathfind_one_step();
        } while (in_execute_event_routine_flag != NO ||
                 triggered_event_list[0] == ACTION_NONE ||
                 g_lcyof != NO ||
                 intro_sequence_active != NO ||
                 lcp_on_stairs_flag != NO ||
                 action_interruptible_flag != NO);

        g_wty = 0;
        g_wtx = 0;
        return -1;
}

/* lcp_calc_floor_waypoint: pick the next waypoint given the current
   resident position and the destination.  Same-floor destinations
   route straight to g_wtx/y; cross-floor destinations route
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

        target_floor  = get_floor_number_from_y(g_wty);
        current_floor = get_floor_number_from_y(lcp_y);

        if (current_floor == target_floor) {
                lcp_on_stairs_flag = NO;
                g_wyx = g_wtx;
                g_wyy = g_wty;
                return;
        }

        target_floor    = get_floor_number_from_y(lcp_y);
        stair_index     = (target_floor - 1) + (target_floor - 1);
        current_floor   = stair_index;
        g_wyx = staircase_waypoint_coords[stair_index];
        g_wyy = staircase_waypoint_coords[current_floor + 1];

        target_floor = get_floor_number_from_y(lcp_y);
        if (target_floor == 2) {
                target_floor = get_floor_number_from_y(g_wty);
                dest_floor   = get_floor_number_from_y(lcp_y);
                if (target_floor < dest_floor) {
                        g_wyx = stair_top_y_threshold;
                        g_wyy = stair_bottom_y_threshold;
                }
        }

        lcp_on_stairs_flag = NO;
        if (lcp_x == g_wyx && lcp_y == g_wyy) {
                lcp_on_stairs_flag = YES;
                if (g_wty < lcp_y) {
                        g_wyx = staircase_waypoint_coords[current_floor + 2];
                        g_wyy = staircase_waypoint_coords[current_floor + 3];
                } else {
                        g_wyy = staircase_waypoint_coords[current_floor - 1];
                        g_wyx = staircase_waypoint_coords[current_floor - 2];
                }
                target_floor = get_floor_number_from_y(lcp_y);
                if (target_floor == 1) {
                        g_wyx = stair_top_y_threshold;
                        g_wyy = stair_bottom_y_threshold;
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

        target_floor  = get_floor_number_from_y(g_dty);
        current_floor = get_floor_number_from_y(dog_y);

        if (current_floor == target_floor) {
                dog_on_stairs_flag = NO;
                g_dyx = g_dtx;
                g_dyy = g_dty;
                return;
        }

        target_floor    = get_floor_number_from_y(dog_y);
        stair_index     = (target_floor - 1) + (target_floor - 1);
        current_floor   = stair_index;
        g_dyx  = staircase_waypoint_coords[stair_index];
        g_dyy  = staircase_waypoint_coords[current_floor + 1];

        target_floor = get_floor_number_from_y(dog_y);
        if (target_floor == 2) {
                target_floor = get_floor_number_from_y(g_dty);
                dest_floor   = get_floor_number_from_y(dog_y);
                if (target_floor < dest_floor) {
                        g_dyx = stair_top_y_threshold - 3;
                        g_dyy = stair_bottom_y_threshold;
                }
        }

        dog_on_stairs_flag = NO;
        if (dog_x == g_dyx && dog_y == g_dyy) {
                target_floor = get_floor_number_from_y(dog_y);
                if (target_floor == 3)
                        dog_x = dog_x - 8;
                dog_on_stairs_flag = YES;
                if (g_dty < dog_y) {
                        g_dyx = staircase_waypoint_coords[current_floor + 2];
                        g_dyy = staircase_waypoint_coords[current_floor + 3];
                } else {
                        g_dyy = staircase_waypoint_coords[current_floor - 1];
                        g_dyx = staircase_waypoint_coords[current_floor - 2];
                }
                target_floor = get_floor_number_from_y(dog_y);
                if (target_floor == 1) {
                        g_dyx = stair_top_y_threshold;
                        g_dyy = stair_bottom_y_threshold;
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
                sf_sele(SFX_FOOTSTEP_STAIRS, 2L);
                return;
        }

        floor = get_floor_number_from_y(lcp_y);
        if (floor == 1) {
                if (lcp_x < 166)
                        sf_sele(SFX_FOOTSTEP_CARPET, 2L);
                else
                        sf_sele(SFX_FOOTSTEP_WOOD, 2L);
        } else if (floor == 2) {
                if (lcp_x > 146 && lcp_x < 234)
                        sf_sele(SFX_FOOTSTEP_CARPET, 2L);
        } else if (floor == 3 && lcp_x > 136) {
                sf_sele(SFX_FOOTSTEP_WOOD, 2L);
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
        if (g_hastl != target) {
                g_hatas = target;
                g_hastl   = target;
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

        if (g_wtx == 0 && g_wty == 0)
                return;

        if (g_wyx == 0 && g_wyy == 0)
                lcp_calc_floor_waypoint();

        /* Exit stair mode when we've reached the target floor. */
        if (lcp_on_stairs_flag != NO) {
                floor_num = get_floor_number_from_y(g_wyy);
                if (lcp_y <= floor_bottom_y_coords[floor_num - 1]) {
                        if (floor_num == 3)
                                lcp_on_stairs_flag = NO;
                        else if (staircase_waypoint_coords[(floor_num - 1) * 2 + 1] <= lcp_y)
                                lcp_on_stairs_flag = NO;
                }
        }

        /* Waypoint reached? */
        if (lcp_x == g_wyx && lcp_y == g_wyy) {
                if (lcp_x == g_wtx && lcp_y == g_wty) {
                        g_wtx = 0;
                        g_wty = 0;
                        lcp_state     = STATE_STAND_IDLE;
                        game_tick_and_animate(0);
                        return;
                }
                lcp_calc_floor_waypoint();
        }

        /* ---- Flat walking (not on stairs) --------------------------- */
        if (lcp_on_stairs_flag == NO) {
                if (g_lcyof != NO)
                        sp_ssco(g_lcieo);

                if (lcp_x < g_wyx) {
                        lcp_facing_direction = FACING_RIGHT;
                        walk_cycle_state();
                        lcp_x = lcp_x + 1;
                        set_head_target(10);
                } else if (g_wyx < lcp_x) {
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

                x_distance = (lcp_x < g_wyx)
                             ? (g_wyx - lcp_x)
                             : (lcp_x - g_wyx);
                if (x_distance < 8) {
                        if (lcp_y < g_wyy)
                                lcp_y = lcp_y + 1;
                        else if (g_wyy < lcp_y)
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
                if (g_wyy < lcp_y) {
                        /* Ascending */
                        if (lcp_y == 161) {
                                if (g_lcyof != NO)
                                        sp_ssco(g_lcieo);
                                lcp_state = STATE_STAIR_CLIMB_FRAME_0;
                                lcp_facing_direction = FACING_LEFT;
                                lcp_x = lcp_x - 6;
                                lcp_y = lcp_y - 2;
                                set_head_target(HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER);
                        } else if (lcp_y == 100) {
                                if (g_lcyof != NO)
                                        sp_ssco(g_lcieo);
                                lcp_state = STATE_STAIR_CLIMB_FRAME_0;
                                lcp_facing_direction = FACING_RIGHT;
                                lcp_x = lcp_x + 3;
                                lcp_y = lcp_y - 2;
                                set_head_target(10);
                        } else if (lcp_y < 162 &&
                                   (lcp_y < 101 || lcp_y > 139)) {
                                if (lcp_y < 100) {
                                        /* Upper flight of stairs, going up-right */
                                        if (g_lcyof != NO)
                                                sp_ssco(g_lcieo);
                                        lcp_facing_direction = FACING_RIGHT;
                                        lcp_y = lcp_y - 1;
                                        next_x = lcp_x;
                                        if (lcp_state != STATE_STAIR_CLIMB_FRAME_3_STEP) {
                                                next_x = lcp_x + 1;
                                                if (next_x != g_wyx)
                                                        next_x = lcp_x + 2;
                                        }
                                        lcp_x = next_x;
                                        stair_climb_cycle();
                                        if (lcp_state == STATE_STAIR_CLIMB_FRAME_3_STEP)
                                                footstep_trigger_flag = YES;
                                        set_head_target(10);
                                } else if (lcp_y < 161) {
                                        /* Lower flight, going up-left */
                                        if (g_lcyof != NO)
                                                sp_ssco(g_lcieo);
                                        lcp_facing_direction = FACING_LEFT;
                                        lcp_y = lcp_y - 1;
                                        next_x = lcp_x;
                                        if (lcp_state != STATE_STAIR_CLIMB_FRAME_3_STEP) {
                                                next_x = lcp_x - 1;
                                                if (next_x != g_wyx)
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
                                if (g_lcyof != NO) {
                                        g_selaf[g_lcieo] = SPRITE_BEHIND_LCP;
                                        sp_upds();
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
                } else if (lcp_y < g_wyy) {
                        /* Descending */
                        if (g_lcyof != NO)
                                sp_ssco(g_lcieo);

                        if (lcp_y == 161) {
                                lcp_state = STATE_STAIR_BTM_FRAME_0;
                                lcp_facing_direction = FACING_RIGHT;
                                lcp_y = 165;
                                lcp_x = lcp_x + 6;
                                set_head_target(8);
                                if (g_lcyof != NO)
                                        sp_ss02(g_lcieo);
                        } else if (lcp_y == 100) {
                                lcp_state = STATE_STAIR_BTM_FRAME_0;
                                lcp_facing_direction = FACING_RIGHT;
                                lcp_y = 102;
                                lcp_x = lcp_x - 2;
                                set_head_target(8);
                                if (g_lcyof != NO)
                                        sp_ss02(g_lcieo);
                        } else if (lcp_y < 162 &&
                                   (lcp_y < 101 || lcp_y > 131)) {
                                if (lcp_y < 100) {
                                        /* Upper flight, going down-left */
                                        if (g_lcyof != NO)
                                                sp_ss02(g_lcieo);
                                        lcp_facing_direction = FACING_LEFT;
                                        lcp_y = lcp_y + 1;
                                        next_x = lcp_x;
                                        if (lcp_state != STATE_STAIR_DESCEND_FRAME_3_STEP) {
                                                next_x = lcp_x - 1;
                                                if (next_x != g_wyx)
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
                                        if (g_lcyof != NO)
                                                sp_ss02(g_lcieo);
                                        lcp_facing_direction = FACING_RIGHT;
                                        lcp_y = lcp_y + 1;
                                        next_x = lcp_x;
                                        if (lcp_state != STATE_STAIR_DESCEND_FRAME_3_STEP) {
                                                next_x = lcp_x + 1;
                                                if (next_x != g_wyx)
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
                                if (g_lcyof != NO)
                                        sp_ss02(g_lcieo);
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
