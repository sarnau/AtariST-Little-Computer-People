/*
 * parts/lcp_path.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x470a, in the 0x400c object). Files under parts/ are never
 * compiled standalone.
 */
/* lcp_path: one 8Hz step along current waypoint.
   Waypoint reached -> done or pick next.  Not on stairs -> flat walk
   toward waypoint (X first, then Y).  On stairs -> stair-phase by Y
   bucket.  Sets fs_trg on the two foot-plant frames.
   addr: lcp_path() */

void
lcp_path()
{
        /* Four locals; the stair branch reuses x_distance for the
           next-X pick, and the last two are written once and never
           read -- leftovers, kept as the original has them. */
        short   x_distance;     /* -2  */
        short   floor_num;      /* -4  */
        short   ani_snap;       /* -6  */
        short   spd_snap;       /* -8  */

        fs_trg = NO;

        if (g_wtx == 0 && g_wty == 0)
                return;

        /* Dead stores.  g_wkadj is referenced exactly once in the
           whole binary -- here. */
        ani_snap = ani_cnt;
        spd_snap = g_wkadj + g_spdc;

        if (g_wyx == 0 && g_wyy == 0)
                lcp_flwp();

        /* Exit stair mode when we've reached the target floor. */
        if (lcp_stR != NO) {
                if (lcp_y <= flr_by[(floor_num = getFlrY(g_wyy)) - 1]) {
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
                } else
                        lcp_flwp();
        }

        /* ---- Flat walking (not on stairs) --------------------------- */
        if (lcp_stR == NO) {
                if (g_lcyof != NO)
                        sp_ssco(g_lcieo);

                if (lcp_x < g_wyx) {
                        lcp_face = FACING_RIGHT;
                        if (lcp_st > STATE_WALK_FRAME_7_STEP)
                                lcp_st = STATE_WALK_FRAME_0;
                        else if (++lcp_st > STATE_WALK_FRAME_7_STEP)
                                lcp_st = STATE_WALK_FRAME_0;
                        lcp_x++;
                        if (g_hastl != 10) {
                                g_hatas = 10;
                                g_hastl = g_hatas;
                        }
                } else if (lcp_x > g_wyx) {
                        lcp_face = FACING_LEFT;
                        if (lcp_st > STATE_WALK_FRAME_7_STEP)
                                lcp_st = STATE_WALK_FRAME_0;
                        else if (++lcp_st > STATE_WALK_FRAME_7_STEP)
                                lcp_st = STATE_WALK_FRAME_0;
                        lcp_x--;
                        if (g_hastl != (HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER)) {
                                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER;
                                g_hastl = g_hatas;
                        }
                } else {
                        if (++lcp_st > STATE_WALK_FRAME_7_STEP)
                                lcp_st = STATE_WALK_FRAME_0;
                }

                if (lcp_x >= g_wyx)
                        x_distance = lcp_x - g_wyx;
                else
                        x_distance = g_wyx - lcp_x;
                if (x_distance < 8) {
                        if (lcp_y < g_wyy)
                                lcp_y++;
                        else if (lcp_y > g_wyy)
                                lcp_y--;
                } else {
                        if (flr_cy[getFlrY(lcp_y) - 1] > lcp_y)
                                lcp_y++;
                        if (flr_cy[getFlrY(lcp_y) - 1] < lcp_y)
                                lcp_y--;
                }

                if (lcp_st == STATE_WALK_FRAME_3_STEP ||
                    lcp_st == STATE_WALK_FRAME_7_STEP)
                        fs_trg = YES;
        }

        /* ---- Stair traversal --------------------------------------- */
        if (lcp_stR != NO) {
                if (lcp_y > g_wyy) {
                        /* Ascending */
                        if (lcp_y == 161) {
                                if (g_lcyof != NO)
                                        sp_ssco(g_lcieo);
                                lcp_st = STATE_STR_CLIMB_F0;   /* STX enters at 9 */
                                lcp_face = FACING_LEFT;
                                lcp_x -= 6;
                                lcp_y -= 2;
                                if (g_hastl != (HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER)) {
                                        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER;
                                        g_hastl = g_hatas;
                                }
                        } else if (lcp_y == 100) {
                                if (g_lcyof != NO)
                                        sp_ssco(g_lcieo);
                                lcp_st = STATE_STR_CLIMB_F0;   /* STX enters at 9 */
                                lcp_face = FACING_RIGHT;
                                lcp_x += 3;
                                lcp_y -= 2;
                                if (g_hastl != 10) {
                                        g_hatas = 10;
                                        g_hastl = g_hatas;
                                }
                        } else if (lcp_y > 161 ||
                                   (lcp_y > 100 && lcp_y < 140)) {
                                /* Top-of-stair frame (state 13..16). */
                                if (g_lcyof != NO) {
                                        g_selaf[g_lcieo] = SPRITE_BEHIND_LCP;
                                        sp_upds();
                                }
                                if (lcp_st < STATE_STR_TOP_F0 || lcp_st > STATE_STR_TOP_F3S) {
                                        lcp_st = STATE_STR_TOP_F0;
                                } else {
                                        /* Wraps to F0, and flips the
                                           facing on the wrap. */
                                        if (++lcp_st > STATE_STR_TOP_F3S) {
                                                lcp_st = STATE_STR_TOP_F0;
                                                lcp_face ^= FACING_LEFT;
                                        }
                                        if (lcp_st == STATE_STR_TOP_F3S ||
                                            lcp_st == STATE_STR_TOP_F0)
                                                lcp_y -= 2;
                                        if (lcp_st == STATE_STR_TOP_F3S)
                                                fs_trg = YES;
                                }
                                if (g_hastl != HEAD_ANIM_HORIZONTAL_RANGE) {
                                        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                                        g_hastl = g_hatas;
                                }
                        } else {
                                if (lcp_y < 100) {
                                        /* Upper flight of stairs, going up-right */
                                        if (g_lcyof != NO)
                                                sp_ssco(g_lcieo);
                                        lcp_face = FACING_RIGHT;
                                        lcp_y--;
                                        if (lcp_st != STATE_STR_CLIMB_F3S) {
                                                lcp_x++;
                                                if (lcp_x != g_wyx)
                                                        lcp_x++;
                                        }
                                        if (++lcp_st > STATE_STR_CLIMB_F3S)
                                                lcp_st = STATE_STR_CLIMB_F0;
                                        if (lcp_st == STATE_STR_CLIMB_F3S)
                                                fs_trg = YES;
                                        if (g_hastl != 10) {
                                                g_hatas = 10;
                                                g_hastl = g_hatas;
                                        }
                                } else if (lcp_y < 161) {
                                        /* Lower flight, going up-left */
                                        if (g_lcyof != NO)
                                                sp_ssco(g_lcieo);
                                        lcp_face = FACING_LEFT;
                                        lcp_y--;
                                        if (lcp_st != STATE_STR_CLIMB_F3S) {
                                                lcp_x--;
                                                if (lcp_x != g_wyx)
                                                        lcp_x--;
                                        }
                                        if (++lcp_st > STATE_STR_CLIMB_F3S)
                                                lcp_st = STATE_STR_CLIMB_F0;
                                        if (lcp_st == STATE_STR_CLIMB_F3S)
                                                fs_trg = YES;
                                        if (g_hastl != (HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER)) {
                                                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER;
                                                g_hastl = g_hatas;
                                        }
                                }
                        }
                } else if (lcp_y < g_wyy) {
                        /* Descending */
                        if (g_lcyof != NO)
                                sp_ssco(g_lcieo);

                        if (lcp_y == 161) {
                                lcp_st = STATE_STR_BTM_F0;
                                lcp_face = FACING_RIGHT;
                                lcp_y += 4;
                                lcp_x += 6;
                                if (g_hastl != 8) {
                                        g_hatas = 8;
                                        g_hastl = g_hatas;
                                }
                                if (g_lcyof != NO)
                                        sp_ss02(g_lcieo);
                        } else if (lcp_y == 100) {
                                lcp_st = STATE_STR_BTM_F0;
                                lcp_face = FACING_RIGHT;
                                lcp_y += 2;
                                lcp_x -= 2;
                                if (g_hastl != 8) {
                                        g_hatas = 8;
                                        g_hastl = g_hatas;
                                }
                                if (g_lcyof != NO)
                                        sp_ss02(g_lcieo);
                        } else if (lcp_y > 161 ||
                                   (lcp_y > 100 && lcp_y < 132)) {
                                /* Bottom-of-stair frame (state 21..24). */
                                if (g_lcyof != NO)
                                        sp_ss02(g_lcieo);
                                if (lcp_st < STATE_STR_BTM_F0 || lcp_st > STATE_STR_BTM_F3) {
                                        lcp_st = STATE_STR_BTM_F0;
                                        lcp_x += 2;
                                } else {
                                        if (++lcp_st > STATE_STR_BTM_F3) {
                                                lcp_st = STATE_STR_BTM_F0;
                                                lcp_face ^= FACING_LEFT;
                                        }
                                        if (lcp_st == STATE_STR_BTM_F1 ||
                                            lcp_st == STATE_STR_BTM_F2)
                                                lcp_y += 2;
                                        if (lcp_st == STATE_STR_BTM_F3)
                                                fs_trg = YES;
                                }
                                if (g_hastl != 8) {
                                        g_hatas = 8;
                                        g_hastl = g_hatas;
                                }
                        } else {
                                if (lcp_y < 100) {
                                        /* Upper flight, going down-left */
                                        if (g_lcyof != NO)
                                                sp_ss02(g_lcieo);
                                        lcp_face = FACING_LEFT;
                                        lcp_y++;
                                        if (lcp_st != STATE_STR_DESC_F3S) {
                                                lcp_x--;
                                                if (lcp_x != g_wyx)
                                                        lcp_x--;
                                        }
                                        /* Wraps to F0, it does not
                                           clamp at F3S. */
                                        if (lcp_st > STATE_STR_DESC_F3S ||
                                            lcp_st < STATE_STR_DESC_F0)
                                                lcp_st = STATE_STR_DESC_F0;
                                        else if (++lcp_st > STATE_STR_DESC_F3S)
                                                lcp_st = STATE_STR_DESC_F0;
                                        if (g_hastl != (HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER)) {
                                                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER;
                                                g_hastl = g_hatas;
                                        }
                                        if (lcp_st == STATE_STR_DESC_F1)
                                                fs_trg = YES;
                                } else if (lcp_y < 161) {
                                        /* Lower flight, going down-right */
                                        if (g_lcyof != NO)
                                                sp_ss02(g_lcieo);
                                        lcp_face = FACING_RIGHT;
                                        lcp_y++;
                                        if (lcp_st != STATE_STR_DESC_F3S) {
                                                lcp_x++;
                                                if (lcp_x != g_wyx)
                                                        lcp_x++;
                                        }
                                        /* Wraps to F0, it does not
                                           clamp at F3S. */
                                        if (lcp_st > STATE_STR_DESC_F3S ||
                                            lcp_st < STATE_STR_DESC_F0)
                                                lcp_st = STATE_STR_DESC_F0;
                                        else if (++lcp_st > STATE_STR_DESC_F3S)
                                                lcp_st = STATE_STR_DESC_F0;
                                        if (g_hastl != 10) {
                                                g_hatas = 10;
                                                g_hastl = g_hatas;
                                        }
                                        if (lcp_st == STATE_STR_DESC_F1)
                                                fs_trg = YES;
                                }
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
