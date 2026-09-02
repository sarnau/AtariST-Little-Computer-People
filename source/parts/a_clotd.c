/*
 * parts/a_clotd.c -- shared body; LCP_ORG links it in adoors.c,
 * LCP_STX in the 0xdece object (0x10556, immediately after a_uset).  Files under parts/
 * are never compiled standalone.
 */

void
a_clotd()
{
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        lcp_face = FACING_LEFT;
        lcp_st = STATE_BEND_AND_REACH;
        gameTick(2);
        od_draw(od_too1, 187, 87);
        gameTick(2);
        od_draw(od_tocl, 187, 87);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        lcp_toiO = NO;

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
