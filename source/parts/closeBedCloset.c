/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* closeBedCloset: close the bedroom closet.  Assumes the resident is already
   standing at it: he turns to face the screen, reaches, the door is
   redrawn ajar then shut with the door-close sound, and lcp_clsO is
   cleared.  Used by cleanUp and at the end of changeClothes. */
void
closeBedCloset()
{
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();

        lcp_face = FACING_LEFT;
        lcp_st = STATE_BEND_AND_REACH;
        gameTick(2);
        drawObject(OBJ_DOOR_CLOSET_OPEN_1, CLOSET_DOOR_X, CLOSET_DOOR_Y);
        gameTick(2);
        drawObject(OBJ_DOOR_CLOSET_CLOSED, CLOSET_DOOR_X, CLOSET_DOOR_Y);
        sfxSelect(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        lcp_clsO = NO;

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
