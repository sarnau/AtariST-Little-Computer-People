/*
 * parts/lcp_path.c -- shared body; LCP_ORG links it in walk.c,
 * LCP_STX in the 0xdece object (0x470a, in the 0x400c object).  Files under parts/
 * are never compiled standalone.
 */
/* lcp_path: one 8Hz step along current waypoint.
   Waypoint reached -> done or pick next.  Not on stairs -> flat walk
   toward waypoint (X first, then Y).  On stairs -> stair-phase by Y
   bucket.  Sets fs_trg on the two foot-plant frames.
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
                        lcp_st = lcp_st + STATE_WALK_FRAME_1;
                        if (lcp_st > STATE_WALK_FRAME_7_STEP)
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
                                lcp_st = STATE_STR_CLIMB_F3S;  /* ROM: enters at 12 */
                                lcp_face = FACING_LEFT;
                                lcp_x = lcp_x - 6;
                                lcp_y = lcp_y - 2;
                                setHTgt(HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER);
                        } else if (lcp_y == 100) {
                                if (g_lcyof != NO)
                                        sp_ssco(g_lcieo);
                                lcp_st = STATE_STR_CLIMB_F3S;  /* ROM: enters at 12 */
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
                                if (lcp_st < STATE_STR_TOP_F0 || lcp_st > STATE_STR_TOP_F3S) {
                                        lcp_st = STATE_STR_TOP_F3S;     /* ROM: snaps to 16 */
                                } else {
                                        lcp_st = lcp_st + STATE_WALK_FRAME_1;
                                        if (lcp_st > STATE_STR_TOP_F3S) {
                                                lcp_st = STATE_STR_TOP_F3S;     /* ROM: 16 */
                                                lcp_face =
                                                        lcp_face ^ FACING_LEFT;
                                        }
                                        /* ROM tests F3S twice (dead
                                           second arm, kept verbatim) */
                                        if (lcp_st == STATE_STR_TOP_F3S ||
                                            lcp_st == STATE_STR_TOP_F3S)
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
                                lcp_st = STATE_STR_BTM_F3;      /* ROM: 24 */
                                lcp_face = FACING_RIGHT;
                                lcp_y = 165;
                                lcp_x = lcp_x + 6;
                                setHTgt(8);
                                if (g_lcyof != NO)
                                        sp_ss02(g_lcieo);
                        } else if (lcp_y == 100) {
                                lcp_st = STATE_STR_BTM_F3;      /* ROM: 24 */
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
                                        if (lcp_st < STATE_STR_BTM_F0 && lcp_st > STATE_STR_TOP_F3S) {
                                                lcp_st = lcp_st + STATE_WALK_FRAME_1;
                                                if (lcp_st > STATE_STR_DESC_F3S)
                                                        lcp_st = STATE_STR_DESC_F3S;
                                        } else {
                                                lcp_st = STATE_STR_DESC_F3S;
                                        }
                                        setHTgt(HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER);
                                        if (lcp_st == STATE_STR_DESC_F3S)
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
                                        if (lcp_st < STATE_STR_BTM_F0 && lcp_st > STATE_STR_TOP_F3S) {
                                                lcp_st = lcp_st + STATE_WALK_FRAME_1;
                                                if (lcp_st > STATE_STR_DESC_F3S)
                                                        lcp_st = STATE_STR_DESC_F3S;
                                        } else {
                                                lcp_st = STATE_STR_DESC_F3S;
                                        }
                                        setHTgt(10);
                                        if (lcp_st == STATE_STR_DESC_F3S)
                                                fs_trg = YES;
                                }
                        } else {
                                /* Bottom-of-stair frame (state 21..24). */
                                if (g_lcyof != NO)
                                        sp_ss02(g_lcieo);
                                if (lcp_st < STATE_STR_BTM_F0 || lcp_st > STATE_STR_BTM_F3) {
                                        lcp_st = STATE_STR_BTM_F3;      /* ROM: 24 */
                                        lcp_x = lcp_x + 2;
                                } else {
                                        lcp_st = lcp_st + STATE_WALK_FRAME_1;
                                        if (lcp_st > STATE_STR_BTM_F3) {
                                                lcp_st = STATE_STR_BTM_F3;      /* ROM: 24 */
                                                lcp_face =
                                                        lcp_face ^ FACING_LEFT;
                                        }
                                        /* ROM tests F3 twice (dead second arm) */
                                        if (lcp_st == STATE_STR_BTM_F3 ||
                                            lcp_st == STATE_STR_BTM_F3)
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
