/* Close the toilet door.  Assumes the resident is already
   at it: he faces the screen, reaches, the door is redrawn ajar then
   shut with the door-close sound, and toiletDoorOpen is cleared.  Used by
   cleanUp and by the toilet routine useToilet. */
void
closeToiletDoor()
{
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        resFacing = FACING_LEFT;
        animState = STATE_BEND_AND_REACH;
        gameTick(2);
        drawObject(OBJ_DOOR_TOILET_OPEN_1, TOILET_DOOR_X, TOILET_DOOR_Y);
        gameTick(2);
        drawObject(OBJ_DOOR_TOILET_CLOSED, TOILET_DOOR_X, TOILET_DOOR_Y);
        sfxSelect(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        toiletDoorOpen = NO;

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
