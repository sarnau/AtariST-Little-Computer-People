/*
 * parts/a_clotd.c -- included by stx_u2.c; never compiled on its own.
 */

/* a_clotd: close the toilet door.  Assumes the resident is already
   at it: he faces the screen, reaches, the door is redrawn ajar then
   shut with the door-close sound, and lcp_toiO is cleared.  Used by
   a_cleau and by the toilet routine a_uset. */
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
