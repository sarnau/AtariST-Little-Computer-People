/* Open (DOOR_OPEN) or close (DOOR_CLOSE) the front door
   while the resident stands at it.  Opening reaches out, draws the
   door ajar then wide with the door-open sound and sets frontDoorOpen;
   closing draws it ajar then shut with the door-close sound and
   clears frontDoorOpen.  A request matching the current state returns
   immediately. */
void
openFrontDoor(request)
short   request;
{
        if (request == DOOR_OPEN) {
                if (frontDoorOpen != NO)
                        return;
                resFacing = FACING_RIGHT;
                animState = STATE_BEND_AND_REACH;
                gameTick(2);
                drawObject(OBJ_DOOR_FRONT_OPEN_1, FRONT_DOOR_X, FRONT_DOOR_Y);
                sfxSelect(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                drawObject(OBJ_DOOR_FRONT_OPEN_2, FRONT_DOOR_X, FRONT_DOOR_Y);
                gameTick(2);
                frontDoorOpen = YES;
        } else if (request != DOOR_OPEN) {      /* redundant re-test, kept on purpose */
                if (frontDoorOpen == NO)
                        return;
                drawObject(OBJ_DOOR_FRONT_OPEN_1, FRONT_DOOR_X, FRONT_DOOR_Y);
                gameTick(2);
                drawObject(OBJ_DOOR_FRONT_CLOSED, FRONT_DOOR_X, FRONT_DOOR_Y);
                sfxSelect(SFX_DOOR_CLOSE, 6L);
                gameTick(2);
                frontDoorOpen = NO;
        }
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
