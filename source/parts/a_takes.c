/*
 * parts/a_takes.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from abathrm.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in abathrm.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_takes()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short   result;
        short   count;
        short   pick;
#else
        short   count;
#endif

        hs_posXY(POS_MID_SHOWER_DOOR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif

        hs_posXY(POS_MID_SHOWER_INSIDE,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_SHOWER_STAND;
#ifdef FAITHFUL
        lcp_x = lcp_x - 8;
#else
        lcp_x -= 8;
#endif
#ifdef FAITHFUL
        lcp_y = lcp_y - 23;
#else
        lcp_y -= 23;
#endif
        g_hatas = 8;
        lcp_hwt();
        g_hamod = HEAD_ANIM_SHOWER;

        count = rndRng(20, 25);
#ifdef FAITHFUL
        while (count != 0) {
#else
        while (count-- != 0) {          /* STX: post-decrement test */
#endif
#ifdef FAITHFUL
                pick = rndRng(0, 1);
                if (pick == 0) {
                        lcp_st = STATE_SHR_SCRUB_L;  gameTick(2);
                        lcp_st = STATE_SHR_SCRUB_R; gameTick(2);
                        lcp_st = STATE_SHR_SCRUB_L;  gameTick(2);
                        lcp_st = STATE_SHR_SCRUB_R; gameTick(2);
                        lcp_st = STATE_SHOWER_STAND;       gameTick(4);
                } else {
                        lcp_st = STATE_SHR_WASH_L;   gameTick(2);
                        lcp_st = STATE_SHR_WASH_R;  gameTick(2);
                        lcp_st = STATE_SHR_WASH_L;   gameTick(2);
                        lcp_st = STATE_SHR_WASH_R;  gameTick(2);
                        lcp_st = STATE_SHOWER_STAND;       gameTick(4);
                }
#else
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
#endif
#ifdef FAITHFUL
                count = count - 1;
#endif
        }

        lcp_st = STATE_STAND_FACING_SCREEN;
#ifdef FAITHFUL
        lcp_y = lcp_y + 29;
#else
        lcp_y += 29;
#endif
        gameTick(2);
        hs_posXY(POS_MID_SHOWER_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();
        g_hamod = HEAD_ANIM_DISABLED;
        g_actif = NO;
}
