/*
 * Included by stx_u1.c; never compiled on its own.
 */
/* dogNextWaypt: dog waypoint math.  Same shape as nextWaypoint but uses
   dog_x/y and applies -3 X on middle-floor landing + -8 X on stair
   crest. */

void
dogNextWaypt()
{
        /* One local: every floor lookup is called inline (the first
           result goes on the stack for the compare).  The equal case
           is the ELSE arm, so its three assignments sit at the end.
           Alcyon evaluates the RIGHT operand first, so swapping the two
           sides of the first comparison changes the compiled code. */
        short   si;

        if (floorOfY(dog_y) != floorOfY(g_dty)) {
                g_dyx = stair_wp[si = (floorOfY(dog_y) - 1) * 2];
                g_dyy = stair_wp[si + 1];

                if (floorOfY(dog_y) == FLOOR_MIDDLE) {
                        if (floorOfY(dog_y) > floorOfY(g_dty)) {
                                g_dyx = stair_ty - 3;
                                g_dyy = stair_by;
                        }
                }

                dg_stair = NO;
                if (dog_x == g_dyx && dog_y == g_dyy) {
                        if (floorOfY(dog_y) == FLOOR_TOP)
                                dog_x -= 8;
                        dg_stair = YES;
                        if (dog_y > g_dty) {
                                g_dyx = stair_wp[si + 2];
                                g_dyy = stair_wp[si + 3];
                        } else {
                                g_dyy = stair_wp[si - 1];
                                g_dyx = stair_wp[si - 2];
                        }
                        if (floorOfY(dog_y) == FLOOR_BOTTOM) {
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
