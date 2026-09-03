/*
 * parts/a_nodh.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from asimple.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in asimple.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_nodh()
{
        short   saved_frame;

        pst_arr[0]  = STATE_WALK_FRAME_3_STEP;
        pst_arr[1]  = STATE_WALK_FRAME_4;
        pst_arr[2]  = STATE_WALK_FRAME_5;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_hwt();

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        g_hsfra = pst_arr[0];
        gameTick(1);
        g_hsfra = pst_arr[1];
        gameTick(1);
        g_hsfra = pst_arr[2];
        gameTick(2);

        g_hatas = 8;
        g_hacur      = 8;
        g_hsfra      = saved_frame;
        gameTick(0);
}
