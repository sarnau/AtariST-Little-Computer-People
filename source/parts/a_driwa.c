/*
 * parts/a_driwa.c -- the resident stands at the kitchen sink with the
 * water running.
 * Included by stx_u2.c; never compiled on its own.
 */

void
a_driwa(value)
short   value;
{
        /* Declaration order matters: it sets the stack-frame layout,
           which must match the original. */
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

        rnd = (unsigned short)(Random() & 0x1f) | 4;
        sf_sele(SFX_WATER_RUNNING, 10000L);

        /* last_pick is never initialised, so the first comparison
           reads whatever the stack slot held.  That is how the 1985
           code is written; do not "fix" it. */
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
