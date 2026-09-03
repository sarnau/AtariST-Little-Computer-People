/*
 * parts/a_sitae.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * aleisure functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_sitae()
{
        short           result;
        /* STX uses signed shorts here: no zero-extension around the
           index arithmetic. */
        /* STX's first local (frame -2) holds the per-frame tick
           count, set to 3 before the loop and passed to gameTick;
           duration and i follow at -4 and -6. */
        short           duration;
        short           i;

        pst_arr[0] = STATE_EX_ARMS_CTR;
        pst_arr[1] = STATE_EX_ARMS_UP;
        pst_arr[2] = STATE_EX_ARMS_CTR;
        pst_arr[3] = STATE_EX_ARMS_WIDE;

        hs_posXY(POS_MID_COUCH,
                              &g_wtx, &g_wty);
        /* STX: -= straight to memory, and the walk call is tested
           inline rather than through a local. */
        g_wty -= 5;
        if (lcp_wkD() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();

        /* STX folds the mask into the assignment (computing it once)
           and increments with i++. */
        result = 3;
        duration = (unsigned short)(Random() & 0x7f) | 8;
        i = 0;
        while (i < duration) {
                if (g_trel[0] != ACTION_NONE)
                        break;
                lcp_st = pst_arr[i & 3];
                if (lcp_st == STATE_EX_ARMS_CTR)
                        gameTick(0);
                else
                        gameTick(result);
                i++;
        }
        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
