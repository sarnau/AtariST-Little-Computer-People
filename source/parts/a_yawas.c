/*
 * parts/a_yawas.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from asimple.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in asimple.c.
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

        /* LCP_ORG's source uses the register form; the STX revision
           writes i++ (addq straight to the frame slot). */
#ifdef FAITHFUL
        for (i = 0; i < 15; i = i + 1) {
#else
        for (i = 0; i < 15; i++) {
#endif
                lcp_st = pst_arr[i & 1];
                gameTick(1);
        }
        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
