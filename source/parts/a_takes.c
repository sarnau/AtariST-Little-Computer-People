/*
 * parts/a_takes.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * abathrm functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_takes()
{
        /* STX tests the call in place -- no local. */
        short   count;

        hs_posXY(POS_MID_SHOWER_DOOR,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        hs_posXY(POS_MID_SHOWER_INSIDE,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_SHOWER_STAND;
        lcp_x -= 8;
        lcp_y -= 23;
        g_hatas = 8;
        lcp_hwt();
        g_hamod = HEAD_ANIM_SHOWER;

        count = rndRng(20, 25);
        while (count-- != 0) {          /* STX: post-decrement test */
                /* STX: call tested in place, arms in the other order. */
                if (rndRng(0, 1) != 0) {
                        lcp_st = STATE_SHR_WASH_L;   gameTick(2);
                        lcp_st = STATE_SHR_WASH_R;  gameTick(2);
                        lcp_st = STATE_SHR_WASH_L;   gameTick(2);
                        lcp_st = STATE_SHR_WASH_R;  gameTick(2);
                        lcp_st = STATE_SHOWER_STAND;       gameTick(4);
                } else {
                        lcp_st = STATE_SHR_SCRUB_L;  gameTick(2);
                        lcp_st = STATE_SHR_SCRUB_R; gameTick(2);
                        lcp_st = STATE_SHR_SCRUB_L;  gameTick(2);
                        lcp_st = STATE_SHR_SCRUB_R; gameTick(2);
                        lcp_st = STATE_SHOWER_STAND;       gameTick(4);
                }
        }

        lcp_st = STATE_STAND_FACING_SCREEN;
        lcp_y += 29;
        gameTick(2);
        hs_posXY(POS_MID_SHOWER_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();
        g_hamod = HEAD_ANIM_DISABLED;
        g_actif = NO;
}
