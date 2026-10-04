/*
 * parts/a_nodh.c -- included by stx_u2.c; never compiled on its own.
 */

/* a_nodh: nod the head.  The resident turns side-on, his head is
   settled at position 8, then head animation is suspended (g_hacur =
   g_hatas = -1) so three head frames can be forced through g_hsfra in
   turn.  The saved frame and head position 8 are restored afterwards. */
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
