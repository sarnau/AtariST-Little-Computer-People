/*
 * parts/a_opecf.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from adoors.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in adoors.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_opecf()
{
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        lcp_face = FACING_LEFT;
        lcp_st = STATE_REACH_INTO_CABINET;
        od_draw(od_fdcl, 24, 153);
        gameTick(1);
        od_draw(od_fdo1, 24, 153);
        sf_sele(SFX_DOOR_OPEN, 6L);
        gameTick(1);
        od_draw(od_fdo2, 24, 153);
        gameTick(1);

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(2);

        lcp_face = FACING_LEFT;
        lcp_st = STATE_REACH_INTO_CABINET;
        gameTick(3);

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(8);

        od_draw(od_fdo1, 24, 153);
        gameTick(1);
        od_draw(od_fdcl, 24, 153);
        sf_sele(SFX_DOOR_OPEN, 6L);   /* verbatim */
        gameTick(1);
}
