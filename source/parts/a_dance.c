/*
 * parts/a_dance.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * ahouse functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_dance()
{
        /* STX has one local (the loop counter); the walk result is
           tested in place. */
        short   i;

        pst_arr[0] = STATE_DANCE_STEP_LEFT;
        pst_arr[1] = STATE_DANCE_STEP_RIGHT;

        if (lcp_recP == NO) {
                g_actif = YES;
                a_lists();
        }
        g_actif = NO;

        hs_posXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        g_wty += 8;
        if (lcp_wkD() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();

        /* STX never initialises i -- the first iteration reads
           whatever the frame slot held.  Preserved as written. */
        while (mi_play != NO) {
                i++;
                lcp_st = pst_arr[i & 1];
                if (g_trel[0] != ACTION_NONE)
                        break;
                gameTick(2);
        }

        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
