/*
 * parts/closeToiletDoor.c -- included by stx_u2.c; never compiled on its own.
 */

/* closeToiletDoor: close the toilet door.  Assumes the resident is already
   at it: he faces the screen, reaches, the door is redrawn ajar then
   shut with the door-close sound, and lcp_toiO is cleared.  Used by
   cleanUp and by the toilet routine useToilet. */
void
closeToiletDoor()
{
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();

        lcp_face = FACING_LEFT;
        lcp_st = STATE_BEND_AND_REACH;
        gameTick(2);
        drawObject(OBJ_DOOR_TOILET_OPEN_1, TOILET_DOOR_X, TOILET_DOOR_Y);
        gameTick(2);
        drawObject(OBJ_DOOR_TOILET_CLOSED, TOILET_DOOR_X, TOILET_DOOR_Y);
        sfxSelect(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        lcp_toiO = NO;

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
