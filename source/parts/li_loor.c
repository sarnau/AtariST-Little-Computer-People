/*
 * parts/li_loor.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * ahouse functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
li_loor()
{
        /* Three locals LCP_STX never references -- the frame is
           link #-10 where li_lool's is #-4; the 1985 source carried
           two copies of this gesture with different declarations. */
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
