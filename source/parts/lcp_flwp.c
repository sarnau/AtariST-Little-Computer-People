/*
 * parts/lcp_flwp.c -- included by stx_u1.c; never compiled on its own.
 * It must sit directly before getFlrY so the call to it stays short.
 */
/* lcp_flwp: pick next waypoint.  Same-floor -> straight to g_wtx/y;
   cross-floor -> through stair_wp[].  Middle floor has an extra
   stair_ty/stair_by landing branch top/bottom don't need. */

void
lcp_flwp()
{
        /* One local: getFlrY is re-called at every use site and the
           stair-table index is assigned inside the first subscript,
           as in the original. */
        short   stair_index;

        if (getFlrY(lcp_y) != getFlrY(g_wty)) {
                g_wyx = stair_wp[stair_index =
                                 (getFlrY(lcp_y) - 1) * 2];
                g_wyy = stair_wp[stair_index + 1];

                if (getFlrY(lcp_y) == FLOOR_MIDDLE)
                        if (getFlrY(lcp_y) > getFlrY(g_wty)) {
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
                        if (getFlrY(lcp_y) == FLOOR_BOTTOM) {
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
