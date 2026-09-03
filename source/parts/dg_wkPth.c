/*
 * parts/dg_wkPth.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x4586, immediately after dg_mvAni). Files under parts/ are
 * never compiled standalone.
 */
/* dg_wkPth: dog waypoint math.  Same shape as lcp_flwp but uses
   dog_x/y and applies -3 X on middle-floor landing + -8 X on stair
   crest.
   addr: dg_wkPth() */

void
dg_wkPth()
{
        /* One local: every floor lookup is called inline (the first
           result goes on the stack for the compare).  The equal case
           is the ELSE arm, so its three assignments sit at the end. */
        short   si;

        if (getFlrY(g_dty) != getFlrY(dog_y)) {
                g_dyx = stair_wp[si = (getFlrY(dog_y) - 1) * 2];
                g_dyy = stair_wp[si + 1];

                if (getFlrY(dog_y) == 2) {
                        if (getFlrY(dog_y) > getFlrY(g_dty)) {
                                g_dyx = stair_ty - 3;
                                g_dyy = stair_by;
                        }
                }

                dg_stair = NO;
                if (dog_x == g_dyx && dog_y == g_dyy) {
                        if (getFlrY(dog_y) == 3)
                                dog_x -= 8;
                        dg_stair = YES;
                        if (dog_y > g_dty) {
                                g_dyx = stair_wp[si + 2];
                                g_dyy = stair_wp[si + 3];
                        } else {
                                g_dyy = stair_wp[si - 1];
                                g_dyx = stair_wp[si - 2];
                        }
                        if (getFlrY(dog_y) == 1) {
                                g_dyx = stair_ty;
                                g_dyy = stair_by;
                        }
                }
        } else {
                dg_stair = NO;
                g_dyx = g_dtx;
                g_dyy = g_dty;
        }
}
