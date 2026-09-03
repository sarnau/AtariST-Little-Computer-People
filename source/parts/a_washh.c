/*
 * parts/a_washh.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from abathrm.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in abathrm.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_washh()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short           result;
#endif
        /* STX declares them rnd, counter, last_pick, val, all
           signed -- the same layout as a_driwa. */
#ifdef FAITHFUL
        unsigned short  rnd;
        unsigned short  val;
        unsigned short  last_pick;
        short           counter;
#else
        short           rnd;
        short           counter;
        short           last_pick;
        short           val;
#endif

        pst_arr[0] = STATE_WASH_HANDS_CENTER;
        pst_arr[1] = STATE_WASH_HANDS_LEFT;
        pst_arr[2] = STATE_WASH_HANDS_RIGHT;

        hs_posXY(POS_MID_BATHROOM_SINK,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

#ifdef FAITHFUL
        rnd = (unsigned short) Random();
#else
        rnd = (unsigned short)(Random() & 0x7f) | 4;
#endif
        sf_sele(SFX_WATER_RUNNING, 10000L);

#ifdef FAITHFUL
        counter   = 0;
        last_pick = 0;
        while (counter < (short) ((rnd & 0x7f) | 4) &&
               g_trel[0] == ACTION_NONE) {
                val = (unsigned short) Random();
                while ((val & 3) == last_pick)
                        val = (unsigned short) Random();
                val = val & 3;
                last_pick = val;
                if (val == 3)
                        lcp_st = pst_arr[1];
                else
                        lcp_st = pst_arr[val];
                lcp_face = (val == 3) ? FACING_LEFT : FACING_RIGHT;
                gameTick(1);
                counter = counter + 1;
        }
#else
        /* last_pick is never initialised in STX (as in a_driwa);
           the counter is. */
        counter = 0;
        while (counter < rnd) {
                if (g_trel[0] != ACTION_NONE)
                        break;
                val = Random() & 3;
                while (val == last_pick)
                        val = Random() & 3;
                last_pick = val;
                if (val != 3) {
                        lcp_st = pst_arr[val];
                        lcp_face = FACING_RIGHT;
                } else {
                        lcp_st = pst_arr[1];
                        lcp_face = FACING_LEFT;
                }
                gameTick(1);
                counter++;
        }
#endif

        if (g_sfplf != NO &&
            g_sfpli == SFX_WATER_RUNNING)
                sf_so();

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
