/*
 * parts/nextWaypoint.c -- included by stx_u1.c; never compiled on its own.
 * It must sit directly before floorOfY so the call to it stays short.
 */
/* nextWaypoint: pick next waypoint.  Same-floor -> straight to g_wtx/y;
   cross-floor -> through stair_wp[].  Middle floor has an extra
   stair_ty/stair_by landing branch top/bottom don't need. */

void
nextWaypoint()
{
        /* One local: floorOfY is re-called at every use site and the
           stair-table index is assigned inside the first subscript,
           as in the original. */
        short   stair_index;

        if (floorOfY(lcp_y) != floorOfY(g_wty)) {
                g_wyx = stair_wp[stair_index =
                                 (floorOfY(lcp_y) - 1) * 2];
                g_wyy = stair_wp[stair_index + 1];

                if (floorOfY(lcp_y) == FLOOR_MIDDLE)
                        if (floorOfY(lcp_y) > floorOfY(g_wty)) {
                                g_wyx = stair_ty;
                                g_wyy = stair_by;
                        }

                lcp_stR = NO;
                if (lcp_x == g_wyx && lcp_y == g_wyy) {
                        lcp_stR = YES;
                        if (lcp_y > g_wty) {
                                g_wyx = stair_wp[stair_index + 2];
                                g_wyy = stair_wp[stair_index + 3];
                        } else {
                                g_wyy = stair_wp[stair_index - 1];
                                g_wyx = stair_wp[stair_index - 2];
                        }
                        if (floorOfY(lcp_y) == FLOOR_BOTTOM) {
                                g_wyx = stair_ty;
                                g_wyy = stair_by;
                        }
                }
        } else {
                lcp_stR = NO;
                g_wyx = g_wtx;
                g_wyy = g_wty;
        }
}
