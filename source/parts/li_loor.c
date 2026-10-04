/*
 * Included by stx_u2.c; never compiled on its own.
 */

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
