/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* ACTION_WASH_HANDS.  The resident walks to the bathroom sink (giving
   up if interrupted), turns to the screen and starts SFX_WATER_RUNNING,
   then plays 4..127 one-tick hand-washing poses picked at random --
   centre, left, right, or the left pose mirrored -- never the same
   pick twice running.  A queued action ends it early.  The water
   sound is stopped if it is still the effect playing. */
void
a_washh()
{
        /* The walk call is tested in place, with no local.  The locals
           are declared in this order, all signed, as in a_driwa. */
        short           rnd;
        short           counter;
        short           last_pick;
        short           val;

        pst_arr[0] = STATE_WASH_HANDS_CENTER;
        pst_arr[1] = STATE_WASH_HANDS_LEFT;
        pst_arr[2] = STATE_WASH_HANDS_RIGHT;

        hs_posXY(POS_MID_BATHROOM_SINK,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        rnd = (unsigned short)(Random() & 0x7f) | 4;
        sf_sele(SFX_WATER_RUNNING, 10000L);

        /* last_pick is never initialised (as in a_driwa), so the first
           comparison reads whatever the slot held.  1985 code, kept on
           purpose. */
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

        if (g_sfplf != NO &&
            g_sfpli == SFX_WATER_RUNNING)
                sf_so();

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
