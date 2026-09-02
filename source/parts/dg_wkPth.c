/*
 * parts/dg_wkPth.c -- shared body; LCP_ORG links it in walk.c,
 * LCP_STX in the 0xdece object (0x4586, immediately after dg_mvAni).  Files under parts/
 * are never compiled standalone.
 */
/* dg_wkPth: dog waypoint math.  Same shape as lcp_flwp but uses
   dog_x/y and applies -3 X on middle-floor landing + -8 X on stair
   crest.
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
