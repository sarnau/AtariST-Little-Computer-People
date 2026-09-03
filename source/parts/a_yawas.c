/*
 * parts/a_yawas.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * asimple functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_yawas()
{
        short   i;

        pst_arr[0]  = STATE_YAWN_MOUTH_OPEN;
        pst_arr[1]  = STATE_YAWN_STRETCH_ARMS;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();

        /* i++ -- addq straight to the frame slot. */
        for (i = 0; i < 15; i++) {
                lcp_st = pst_arr[i & 1];
                gameTick(1);
        }
        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
