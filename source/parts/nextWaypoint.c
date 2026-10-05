/*
 * It must sit directly before floorOfY so the call to it stays short.
 */
/* Pick next waypoint.  Same-floor -> straight to walkXTarget/y;
   cross-floor -> through stairWaypts[].  Middle floor has an extra
   xLanding/yLanding landing branch top/bottom don't need. */

void
nextWaypoint()
{
        /* One local: floorOfY is re-called at every use site and the
           stair-table index is assigned inside the first subscript,
           as in the original. */
        short   stairIndex;

        if (floorOfY(resY) != floorOfY(walkYTarget)) {
                xWaypoint = stairWaypts[stairIndex =
                                 (floorOfY(resY) - 1) * 2];
                yWaypoint = stairWaypts[stairIndex + 1];

                if (floorOfY(resY) == FLOOR_MIDDLE)
                        if (floorOfY(resY) > floorOfY(walkYTarget)) {
                                xWaypoint = xLanding;
                                yWaypoint = yLanding;
                        }

                onStairs = NO;
                if (resX == xWaypoint && resY == yWaypoint) {
                        onStairs = YES;
                        if (resY > walkYTarget) {
                                xWaypoint = stairWaypts[stairIndex + 2];
                                yWaypoint = stairWaypts[stairIndex + 3];
                        } else {
                                yWaypoint = stairWaypts[stairIndex - 1];
                                xWaypoint = stairWaypts[stairIndex - 2];
                        }
                        if (floorOfY(resY) == FLOOR_BOTTOM) {
                                xWaypoint = xLanding;
                                yWaypoint = yLanding;
                        }
                }
        } else {
                onStairs = NO;
                xWaypoint = walkXTarget;
                yWaypoint = walkYTarget;
        }
}
