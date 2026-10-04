/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* a_clocd: close the bedroom closet.  Assumes the resident is already
   standing at it: he turns to face the screen, reaches, the door is
   redrawn ajar then shut with the door-close sound, and lcp_clsO is
   cleared.  Used by a_cleau and at the end of a_opcbc. */
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
