/*
 * parts/lcp_flwp.c -- shared body; LCP_STX puts it at 0x50bc in the
 * 0x400c object, immediately before getFlrY (the call is a bsr).
 * Files under parts/ are never compiled standalone.
 */
/* lcp_flwp: pick next waypoint.  Same-floor -> straight to g_wtx/y;
   cross-floor -> through stair_wp[].  Middle floor has an extra
   stair_ty/stair_by landing branch top/bottom don't need.
   addr: lcp_flwp() */

void
lcp_flwp()
{
        /* One local: STX re-calls getFlrY at every use site and folds
           the stair-table index into the first subscript. */
        short   stair_index;

        if (getFlrY(lcp_y) != getFlrY(g_wty)) {
                g_wyx = stair_wp[stair_index =
                                 (getFlrY(lcp_y) - 1) * 2];
                g_wyy = stair_wp[stair_index + 1];

                if (getFlrY(lcp_y) == 2)
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
                        if (getFlrY(lcp_y) == 1) {
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
