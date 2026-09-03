/*
 * parts/dg_mvAni.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x412c). Files under parts/ are never compiled standalone.
 */
/* dg_mvAni: 8 Hz movement + walk-cycle advance.  If the dog
   has no target the routine is a no-op.  Handles flat walking (X/Y
   equal steps to waypoint) and stair navigation (staircase_waypoint_
   coords[] gate for the two staircase entrances).  Layer depth is
   1 (in-front) when the dog is below the resident, -1 (behind) when
   above -- newspaper reading forces in-front so the dog doesn't disappear
   behind the paper.

   addr: dg_mvAni() */

void
dg_mvAni()
{
        /* STX's frame is -16: six shorts, declared in this order, two
           of them never touched. */
        short   x_distance;
        short   floor_num;
        short   unused1;
        short   next_x;
        short   depth_layer;
        BOOL16  h_flip;

        g_dwanc++;
        if (g_dwanc > 7)
                g_dwanc = 0;

        if (g_dtx == 0 && g_dty == 0)
                return;

        if ((short) (dog_y + 5) <= lcp_y)
                depth_layer = -1;
        else
                depth_layer = 1;
        if (lcp_st == STATE_READ_PAPER_HOLD ||
            lcp_st == STATE_READ_PAPER_TURN_PAGE)
                depth_layer = 1;

        if (g_dyx == 0 && g_dyy == 0)
                dg_wkPth();

        /* Exit stair-mode when reaching a floor boundary. */
        if (dg_stair != NO) {
                /* The assignment is embedded, so the index reuses
                   getFlrY's result in d0. */
                if (dog_y <= flr_by[(floor_num = getFlrY(g_dyy)) - 1]) {
                        if (floor_num == 3)
                                dg_stair = NO;
                        else if (stair_wp[(floor_num - 1) * 2 + 1] <= dog_y)
                                dg_stair = NO;
                }
        }

        if (dog_x == g_dyx && dog_y == g_dyy) {
                if (dog_x == g_dtx && dog_y == g_dty) {
                        g_dtx = 0;
                        g_dty = 0;
                        g_dyx = 0;
                        g_dyy = 0;
                        g_dsid = SPRITE_DOG_LAY_DOWN;
                        sp_spud(g_dsid, depth_layer, NO);
                        return;
                } else
                        dg_wkPth();
        }

        g_dsid = g_dwanf[g_dwanc];

        if (dg_stair == NO) {
                if (dog_x < g_dyx) {
                        h_flip = NO;
                        dog_x++;
                } else if (dog_x > g_dyx) {
                        h_flip = YES;
                        dog_x--;
                }
                /* STX writes an if/else with the store duplicated in
                   both arms, not a ternary. */
                if (dog_x >= g_dyx)
                        x_distance = dog_x - g_dyx;
                else
                        x_distance = g_dyx - dog_x;
                if (x_distance < 8) {
                        if (dog_y < g_dyy)
                                dog_y++;
                        else if (dog_y > g_dyy)
                                dog_y--;
                } else {
                        if (dog_y < flr_cy[getFlrY(dog_y) - 1])
                                dog_y++;
                        if (flr_cy[getFlrY(dog_y) - 1] < dog_y)
                                dog_y--;
                }
        }

        /* STX has no `next_x`: the stair patches step dog_x and
           dog_y in place, and every anchor is written as a relative
           step rather than an absolute coordinate. */
        if (dg_stair != NO) {
                if (dog_y > g_dyy) {
                        /* Going up */
                        if (dog_y == 0xa1) {
                                h_flip = YES;
                                dog_x -= 17;
                                dog_y -= 2;
                        } else if (dog_y == 100) {
                                h_flip = NO;
                                dog_x += 3;
                                dog_y -= 2;
                        } else if (dog_y > 161 ||
                                   (dog_y > 100 && dog_y < 140)) {
                                h_flip = NO;
                                dog_y -= 2;
                        } else if (dog_y < 100) {
                                h_flip = NO;
                                dog_y--;
                                if (g_dsid != SPRITE_DOG_WLK_R9) {
                                        dog_x++;
                                        if (dog_x != g_dyx)
                                                dog_x++;
                                }
                        } else if (dog_y < 0xa1) {
                                h_flip = YES;
                                dog_y--;
                                if (g_dsid != SPRITE_DOG_WLK_R9) {
                                        dog_x--;
                                        if (dog_x != g_dyx)
                                                dog_x--;
                                }
                        }
                } else if (dog_y < g_dyy) {
                        /* Going down */
                        if (dog_y == 0xa1) {
                                h_flip = NO;
                                dog_y += 4;
                                dog_x++;
                        } else if (dog_y == 100) {
                                h_flip = NO;
                                dog_y += 2;
                                dog_x += 3;
                        } else if (dog_y > 161 ||
                                   (dog_y > 100 && dog_y < 132)) {
                                h_flip = NO;
                                dog_y++;
                        } else if (dog_y < 100) {
                                h_flip = YES;
                                dog_y++;
                                if (g_dsid != SPRITE_DOG_WLK_R9) {
                                        dog_x--;
                                        if (dog_x != g_dyx)
                                                dog_x--;
                                }
                        } else if (dog_y < 0xa1) {
                                h_flip = NO;
                                dog_y++;
                                if (g_dsid != SPRITE_DOG_WLK_R9) {
                                        dog_x++;
                                        if (dog_x != g_dyx)
                                                dog_x++;
                                }
                        }
                }
        }

        sp_spud(g_dsid, depth_layer, h_flip);
}
