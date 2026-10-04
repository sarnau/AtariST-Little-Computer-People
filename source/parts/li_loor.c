/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* The resident turns to face the screen, waits for its head to swing
   round (lcp_hwt), bends down for a moment and straightens up again --
   a "stand and look" gesture.  a_lists and a_playp use this copy; tt_on
   and tt_off use the otherwise identical li_lool.  Changes lcp_face,
   lcp_st and the head target g_hatas. */
void
li_loor()
{
        /* Three unused locals, but they must stay: removing them
           changes the compiled code.  The 1985 source carried two
           copies of this gesture (see li_lool) with different
           declarations. */
        short   unused1;
        short   unused2;
        short   unused3;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        lcp_st = STATE_BEND_DOWN;
        gameTick(4);
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
