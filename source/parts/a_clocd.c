/*
 * parts/a_clocd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from adoors.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in adoors.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_clocd()
{
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        lcp_face = FACING_LEFT;
        lcp_st = STATE_BEND_AND_REACH;
        gameTick(2);
        od_draw(od_clo1, 75, 87);
        gameTick(2);
        od_draw(od_clcl, 75, 87);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        lcp_clsO = NO;

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
