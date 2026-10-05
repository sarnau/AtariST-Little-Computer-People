/*
 * parts/openFrontDoor.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* openFrontDoor: open (DOOR_OPEN, 0) or close (DOOR_CLOSE) the front door
   while the resident stands at it.  Opening reaches out, draws the
   door ajar then wide with the door-open sound and sets lcp_frdO;
   closing draws it ajar then shut with the door-close sound and
   clears lcp_frdO.  A request matching the current state returns
   immediately. */
void
openFrontDoor(door_st)
short   door_st;
{
        if (door_st == 0) {
                if (lcp_frdO != NO)
                        return;
                lcp_face = FACING_RIGHT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                drawObject(OBJ_DOOR_FRONT_OPEN_1, FRONT_DOOR_X, FRONT_DOOR_Y);
                sfxSelect(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                drawObject(OBJ_DOOR_FRONT_OPEN_2, FRONT_DOOR_X, FRONT_DOOR_Y);
                gameTick(2);
                lcp_frdO = YES;
        } else if (door_st != 0) {      /* redundant re-test, kept on purpose */
                if (lcp_frdO == NO)
                        return;
                drawObject(OBJ_DOOR_FRONT_OPEN_1, FRONT_DOOR_X, FRONT_DOOR_Y);
                gameTick(2);
                drawObject(OBJ_DOOR_FRONT_CLOSED, FRONT_DOOR_X, FRONT_DOOR_Y);
                sfxSelect(SFX_DOOR_CLOSE, 6L);
                gameTick(2);
                lcp_frdO = NO;
        }
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
