/*
 * walk.c -- LCP & dog pathfinding + step animation.
 *
 * Four functions:
 *   lcp_wkD()   -- outer loop: steps toward walk_target_
 *                                  x/y until arrival or interruption.
 *   lcp_path()     -- inner loop: one 8Hz step, handles flat
 *                                  walking, 3 staircase phases (climb,
 *                                  top-of-flight, descend, bottom-of-
 *                                  flight), footstep sound triggers.
 *   lcp_flwp()   -- when there's no active waypoint,
 *                                  routes the resident to the next
 *                                  staircase entry (or straight to
 *                                  destination if same floor).
 *   dg_wkPth()        -- same waypoint math for the dog.
 *   lcp_fstp()   -- selects carpet/wood/stairs SFX
 *                                  based on floor + X position.
 *
 * addr: lcp_wkD(), lcp_path(),
 *       lcp_flwp(), dg_wkPth(),
 *       lcp_fstp()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "events.h"
#include "globals.h"
#include "movement.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"

/* lcp_wkD: pump lcp_path() until arrival.
   Returns 0 on natural arrival, -1 on interruption (only when the
   resident is idle enough for the queue to preempt).
   addr: lcp_wkD() */

short
lcp_wkD()
{
        g_hamod       = HEAD_ANIM_WALKING;
        g_hastl = 0;

        do {
                if (g_wtx == 0 && g_wty == 0)
                        return 0;
                lcp_path();
        } while (in_evrt != NO ||
                 g_trel[0] == ACTION_NONE ||
                 g_lcyof != NO ||
                 introSeq != NO ||
                 lcp_stR != NO ||
                 g_actif != NO);

        g_wty = 0;
        g_wtx = 0;
        return -1;
}

/* lcp_flwp: pick the next waypoint given the current
   resident position and the destination.  Same-floor destinations
   route straight to g_wtx/y; cross-floor destinations route
   through the appropriate entry of stair_wp[] first.
   The middle-floor case has an extra pair-of-flights landing branch
   (stair_ty / _bottom_y_threshold) that the top and
   bottom floors don't need.

   addr: lcp_flwp() */

void
lcp_flwp()
{
        short   target_floor;
        short   current_floor;
        short   dest_floor;
        short   stair_index;

        target_floor  = getFlrY(g_wty);
        current_floor = getFlrY(lcp_y);

        if (current_floor == target_floor) {
                lcp_stR = NO;
                g_wyx = g_wtx;
                g_wyy = g_wty;
                return;
        }

        target_floor    = getFlrY(lcp_y);
        stair_index     = (target_floor - 1) + (target_floor - 1);
        current_floor   = stair_index;
        g_wyx = stair_wp[stair_index];
        g_wyy = stair_wp[current_floor + 1];

        target_floor = getFlrY(lcp_y);
        if (target_floor == 2) {
                target_floor = getFlrY(g_wty);
                dest_floor   = getFlrY(lcp_y);
                if (target_floor < dest_floor) {
                        g_wyx = stair_ty;
                        g_wyy = stair_by;
                }
        }

        lcp_stR = NO;
        if (lcp_x == g_wyx && lcp_y == g_wyy) {
                lcp_stR = YES;
                if (g_wty < lcp_y) {
                        g_wyx = stair_wp[current_floor + 2];
                        g_wyy = stair_wp[current_floor + 3];
                } else {
                        g_wyy = stair_wp[current_floor - 1];
                        g_wyx = stair_wp[current_floor - 2];
                }
                target_floor = getFlrY(lcp_y);
                if (target_floor == 1) {
                        g_wyx = stair_ty;
                        g_wyy = stair_by;
                }
        }
}

/* dg_wkPth: dog waypoint math.  Structurally identical to
   lcp_flwp but uses dog_x/y/target/waypoint and applies
   a -3 X patch on the middle-floor landing plus an -8 X kick when the
   dog crests the top of a staircase.
   addr: dg_wkPth() */

void
dg_wkPth()
{
        short   target_floor;
        short   current_floor;
        short   dest_floor;
        short   stair_index;

        target_floor  = getFlrY(g_dty);
        current_floor = getFlrY(dog_y);

        if (current_floor == target_floor) {
                dg_stair = NO;
                g_dyx = g_dtx;
                g_dyy = g_dty;
                return;
        }

        target_floor    = getFlrY(dog_y);
        stair_index     = (target_floor - 1) + (target_floor - 1);
        current_floor   = stair_index;
        g_dyx  = stair_wp[stair_index];
        g_dyy  = stair_wp[current_floor + 1];

        target_floor = getFlrY(dog_y);
        if (target_floor == 2) {
                target_floor = getFlrY(g_dty);
                dest_floor   = getFlrY(dog_y);
                if (target_floor < dest_floor) {
                        g_dyx = stair_ty - 3;
                        g_dyy = stair_by;
                }
        }

        dg_stair = NO;
        if (dog_x == g_dyx && dog_y == g_dyy) {
                target_floor = getFlrY(dog_y);
                if (target_floor == 3)
                        dog_x = dog_x - 8;
                dg_stair = YES;
                if (g_dty < dog_y) {
                        g_dyx = stair_wp[current_floor + 2];
                        g_dyy = stair_wp[current_floor + 3];
                } else {
                        g_dyy = stair_wp[current_floor - 1];
                        g_dyx = stair_wp[current_floor - 2];
                }
                target_floor = getFlrY(dog_y);
                if (target_floor == 1) {
                        g_dyx = stair_ty;
                        g_dyy = stair_by;
                }
        }
}

/* lcp_fstp: sample-selecting footstep helper.
   fs_trg is set by lcp_path exactly when
   the walk-cycle frame that plants a foot has just landed.
   addr: lcp_fstp() */

void
lcp_fstp()
{
        short   floor;

        if (fs_trg == NO)
                return;

        if (lcp_stR != NO) {
                sf_sele(SFX_FOOTSTEP_STAIRS, 2L);
                return;
        }

        floor = getFlrY(lcp_y);
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

/* ---- lcp_path -------------------------------------------- */

/* Nested-if helper: cycle the walk state through 0..7. */
static void
wkCyc()
{
        if (lcp_st < 8) {
                lcp_st = lcp_st + STATE_WALK_FRAME_1;
                if (lcp_st > 7)
                        lcp_st = STATE_WALK_FRAME_0;
        } else {
                lcp_st = STATE_WALK_FRAME_0;
        }
}

/* Nested-if helper: cycle stair-climb state 9..12 wrapping. */
static void
stairCyc()
{
        lcp_st = lcp_st + STATE_WALK_FRAME_1;
        if (lcp_st > 12)
                lcp_st = STATE_STR_CLIMB_F0;
}

/* Nested-if helper: set head_anim_target if it isn't already `target`. */
static void
setHTgt(target)
short   target;
{
        if (g_hastl != target) {
                g_hatas = target;
                g_hastl   = target;
        }
}

/* lcp_path: one 8Hz step along the current waypoint.
   Structurally: (1) if waypoint reached, either destination is reached
   (return) or pick the next waypoint.  (2) If not on stairs, flat walk
   toward waypoint (X first with cycle, then Y in "channel" of the
   floor center).  (3) If on stairs, run the appropriate stair phase
   for the current stair Y bucket.  Footstep flag is set on the two
   step-planting frames per walk cycle.

   addr: lcp_path() */

void
lcp_path()
{
        short   floor_num;
        short   x_distance;
        short   next_x;

        fs_trg = NO;

        if (g_wtx == 0 && g_wty == 0)
                return;

        if (g_wyx == 0 && g_wyy == 0)
                lcp_flwp();

        /* Exit stair mode when we've reached the target floor. */
        if (lcp_stR != NO) {
                floor_num = getFlrY(g_wyy);
                if (lcp_y <= flr_by[floor_num - 1]) {
                        if (floor_num == 3)
                                lcp_stR = NO;
                        else if (stair_wp[(floor_num - 1) * 2 + 1] <= lcp_y)
                                lcp_stR = NO;
                }
        }

        /* Waypoint reached? */
        if (lcp_x == g_wyx && lcp_y == g_wyy) {
                if (lcp_x == g_wtx && lcp_y == g_wty) {
                        g_wtx = 0;
                        g_wty = 0;
                        lcp_st     = STATE_STAND_IDLE;
                        gameTick(0);
                        return;
                }
                lcp_flwp();
        }

        /* ---- Flat walking (not on stairs) --------------------------- */
        if (lcp_stR == NO) {
                if (g_lcyof != NO)
                        sp_ssco(g_lcieo);

                if (lcp_x < g_wyx) {
                        lcp_face = FACING_RIGHT;
                        wkCyc();
                        lcp_x = lcp_x + 1;
                        setHTgt(10);
                } else if (g_wyx < lcp_x) {
                        lcp_face = FACING_LEFT;
                        wkCyc();
                        lcp_x = lcp_x - 1;
                        setHTgt(HEAD_ANIM_HORIZONTAL_RANGE |
                                        HEAD_ANIM_SHOWER);
                } else {
                        /* At target X, just cycle animation. */
                        lcp_st = lcp_st + STATE_WALK_FRAME_1;
                        if (lcp_st > 7)
                                lcp_st = STATE_WALK_FRAME_0;
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
                        floor_num = getFlrY(lcp_y);
                        if (lcp_y < flr_cy[floor_num - 1])
                                lcp_y = lcp_y + 1;
                        floor_num = getFlrY(lcp_y);
                        if (flr_cy[floor_num - 1] < lcp_y)
                                lcp_y = lcp_y - 1;
                }

                if (lcp_st == STATE_WALK_FRAME_3_STEP ||
                    lcp_st == STATE_WALK_FRAME_7_STEP)
                        fs_trg = YES;
        }

        /* ---- Stair traversal --------------------------------------- */
        if (lcp_stR != NO) {
                if (g_wyy < lcp_y) {
                        /* Ascending */
                        if (lcp_y == 161) {
                                if (g_lcyof != NO)
                                        sp_ssco(g_lcieo);
                                lcp_st = STATE_STR_CLIMB_F0;
                                lcp_face = FACING_LEFT;
                                lcp_x = lcp_x - 6;
                                lcp_y = lcp_y - 2;
                                setHTgt(HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER);
                        } else if (lcp_y == 100) {
                                if (g_lcyof != NO)
                                        sp_ssco(g_lcieo);
                                lcp_st = STATE_STR_CLIMB_F0;
                                lcp_face = FACING_RIGHT;
                                lcp_x = lcp_x + 3;
                                lcp_y = lcp_y - 2;
                                setHTgt(10);
                        } else if (lcp_y < 162 &&
                                   (lcp_y < 101 || lcp_y > 139)) {
                                if (lcp_y < 100) {
                                        /* Upper flight of stairs, going up-right */
                                        if (g_lcyof != NO)
                                                sp_ssco(g_lcieo);
                                        lcp_face = FACING_RIGHT;
                                        lcp_y = lcp_y - 1;
                                        next_x = lcp_x;
                                        if (lcp_st != STATE_STR_CLIMB_F3S) {
                                                next_x = lcp_x + 1;
                                                if (next_x != g_wyx)
                                                        next_x = lcp_x + 2;
                                        }
                                        lcp_x = next_x;
                                        stairCyc();
                                        if (lcp_st == STATE_STR_CLIMB_F3S)
                                                fs_trg = YES;
                                        setHTgt(10);
                                } else if (lcp_y < 161) {
                                        /* Lower flight, going up-left */
                                        if (g_lcyof != NO)
                                                sp_ssco(g_lcieo);
                                        lcp_face = FACING_LEFT;
                                        lcp_y = lcp_y - 1;
                                        next_x = lcp_x;
                                        if (lcp_st != STATE_STR_CLIMB_F3S) {
                                                next_x = lcp_x - 1;
                                                if (next_x != g_wyx)
                                                        next_x = lcp_x - 2;
                                        }
                                        lcp_x = next_x;
                                        stairCyc();
                                        if (lcp_st == STATE_STR_CLIMB_F3S)
                                                fs_trg = YES;
                                        setHTgt(HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER);
                                }
                        } else {
                                /* Top-of-stair frame (state 13..16). */
                                if (g_lcyof != NO) {
                                        g_selaf[g_lcieo] = SPRITE_BEHIND_LCP;
                                        sp_upds();
                                }
                                if (lcp_st < 13 || lcp_st > 16) {
                                        lcp_st = STATE_STR_TOP_F0;
                                } else {
                                        lcp_st = lcp_st + STATE_WALK_FRAME_1;
                                        if (lcp_st > 16) {
                                                lcp_st = STATE_STR_TOP_F0;
                                                lcp_face =
                                                        lcp_face ^ FACING_LEFT;
                                        }
                                        if (lcp_st == STATE_STR_TOP_F3S ||
                                            lcp_st == STATE_STR_TOP_F0)
                                                lcp_y = lcp_y - 2;
                                        if (lcp_st == STATE_STR_TOP_F3S)
                                                fs_trg = YES;
                                }
                                setHTgt(HEAD_ANIM_HORIZONTAL_RANGE);
                        }
                } else if (lcp_y < g_wyy) {
                        /* Descending */
                        if (g_lcyof != NO)
                                sp_ssco(g_lcieo);

                        if (lcp_y == 161) {
                                lcp_st = STATE_STR_BTM_F0;
                                lcp_face = FACING_RIGHT;
                                lcp_y = 165;
                                lcp_x = lcp_x + 6;
                                setHTgt(8);
                                if (g_lcyof != NO)
                                        sp_ss02(g_lcieo);
                        } else if (lcp_y == 100) {
                                lcp_st = STATE_STR_BTM_F0;
                                lcp_face = FACING_RIGHT;
                                lcp_y = 102;
                                lcp_x = lcp_x - 2;
                                setHTgt(8);
                                if (g_lcyof != NO)
                                        sp_ss02(g_lcieo);
                        } else if (lcp_y < 162 &&
                                   (lcp_y < 101 || lcp_y > 131)) {
                                if (lcp_y < 100) {
                                        /* Upper flight, going down-left */
                                        if (g_lcyof != NO)
                                                sp_ss02(g_lcieo);
                                        lcp_face = FACING_LEFT;
                                        lcp_y = lcp_y + 1;
                                        next_x = lcp_x;
                                        if (lcp_st != STATE_STR_DESC_F3S) {
                                                next_x = lcp_x - 1;
                                                if (next_x != g_wyx)
                                                        next_x = lcp_x - 2;
                                        }
                                        lcp_x = next_x;
                                        if (lcp_st < 21 && lcp_st > 16) {
                                                lcp_st = lcp_st + STATE_WALK_FRAME_1;
                                                if (lcp_st > 20)
                                                        lcp_st = STATE_STR_DESC_F0;
                                        } else {
                                                lcp_st = STATE_STR_DESC_F0;
                                        }
                                        setHTgt(HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER);
                                        if (lcp_st == STATE_STR_DESC_F1)
                                                fs_trg = YES;
                                } else if (lcp_y < 161) {
                                        /* Lower flight, going down-right */
                                        if (g_lcyof != NO)
                                                sp_ss02(g_lcieo);
                                        lcp_face = FACING_RIGHT;
                                        lcp_y = lcp_y + 1;
                                        next_x = lcp_x;
                                        if (lcp_st != STATE_STR_DESC_F3S) {
                                                next_x = lcp_x + 1;
                                                if (next_x != g_wyx)
                                                        next_x = lcp_x + 2;
                                        }
                                        lcp_x = next_x;
                                        if (lcp_st < 21 && lcp_st > 16) {
                                                lcp_st = lcp_st + STATE_WALK_FRAME_1;
                                                if (lcp_st > 20)
                                                        lcp_st = STATE_STR_DESC_F0;
                                        } else {
                                                lcp_st = STATE_STR_DESC_F0;
                                        }
                                        setHTgt(10);
                                        if (lcp_st == STATE_STR_DESC_F1)
                                                fs_trg = YES;
                                }
                        } else {
                                /* Bottom-of-stair frame (state 21..24). */
                                if (g_lcyof != NO)
                                        sp_ss02(g_lcieo);
                                if (lcp_st < 21 || lcp_st > 24) {
                                        lcp_st = STATE_STR_BTM_F0;
                                        lcp_x = lcp_x + 2;
                                } else {
                                        lcp_st = lcp_st + STATE_WALK_FRAME_1;
                                        if (lcp_st > 24) {
                                                lcp_st = STATE_STR_BTM_F0;
                                                lcp_face =
                                                        lcp_face ^ FACING_LEFT;
                                        }
                                        if (lcp_st == STATE_STR_BTM_F1 ||
                                            lcp_st == STATE_STR_BTM_F2)
                                                lcp_y = lcp_y + 2;
                                        if (lcp_st == STATE_STR_BTM_F3)
                                                fs_trg = YES;
                                }
                                setHTgt(8);
                        }
                }
        }

        /* Sickness slows the walk: two ticks per step and delayed
           footstep sound.  Healthy: one tick with immediate sound. */
        if (lcp.sickness_level != SICKNESS_HEALTHY) {
                gameTick(0);
                lcp_fstp();
        }
        gameTick(0);
        if (lcp.sickness_level == SICKNESS_HEALTHY)
                lcp_fstp();
}
