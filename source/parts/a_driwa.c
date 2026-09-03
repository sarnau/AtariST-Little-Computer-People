/*
 * parts/a_driwa.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * abathrm functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_driwa(value)
short   value;
{
        /* STX declares them rnd, counter, last_pick, pick -- the
           frame offsets follow that order. */
        short           rnd;
        short           counter;
        short           last_pick;
        short           pick;

        pst_arr[0] = STATE_WASH_HANDS_CENTER;
        pst_arr[1] = STATE_WASH_HANDS_LEFT;
        pst_arr[2] = STATE_WASH_HANDS_RIGHT;

        sp_ssco(value);
        hs_posXY(POS_BTM_KITCHEN_SINK,
                              &g_wtx, &g_wty);
        lcp_wkD();
        g_selaf[value] = SPRITE_HIDDEN;
        sp_upds();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        /* STX folds the mask into the assignment (computed once). */
        rnd = (unsigned short)(Random() & 0x1f) | 4;
        sf_sele(SFX_WATER_RUNNING, 10000L);

        /* STX masks at the assignment and never initialises
           last_pick -- the first comparison reads whatever the frame
           slot held.  Preserved as the original wrote it. */
        for (counter = 0; counter < rnd; counter++) {
                pick = Random() & 3;
                while (pick == last_pick)
                        pick = Random() & 3;
                last_pick = pick;
                if (pick != 3) {
                        lcp_st = pst_arr[pick];
                        lcp_face = FACING_RIGHT;
                } else {
                        lcp_st = pst_arr[1];
                        lcp_face = FACING_LEFT;
                }
                gameTick(1);
        }

        if (g_sfplf != NO &&
            g_sfpli == SFX_WATER_RUNNING)
                sf_so();

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
