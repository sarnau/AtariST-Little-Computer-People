/*
 * parts/a_takes.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

void
a_takes()
{
        /* lcp_wkD()'s result is tested in place, with no local for it. */
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
        while (count-- != 0) {          /* post-decrement test, on purpose */
                /* The call is tested in place; the arm order is the
                   original's and affects the compiled code. */
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
