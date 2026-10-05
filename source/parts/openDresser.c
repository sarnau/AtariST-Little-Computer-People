/* Open (DOOR_OPEN) or close (DOOR_CLOSE) a dresser drawer
   while the resident stands at it, updating dresserOpen.  He bends and
   reaches while the drawer is drawn half then fully open or shut.
   Plays no sound.  A request matching the current state returns
   immediately. */
void
openDresser(request)
short   request;
{
        if (request == DOOR_OPEN) {
                if (dresserOpen != NO)
                        return;
                dresserOpen = YES;
                animState = STATE_BEND_DOWN;    gameTick(1);
                animState = STATE_REACH_FORWARD;gameTick(2);
                drawObject(OBJ_DRESSER_OPEN_1, DRESSER_X, DRESSER_Y);
                gameTick(2);
                drawObject(OBJ_DRESSER_OPEN_2, DRESSER_X, DRESSER_Y);
                gameTick(2);
        } else if (request != DOOR_OPEN) {      /* redundant re-test, kept on purpose */
                if (dresserOpen == NO)
                        return;
                dresserOpen = NO;
                animState = STATE_BEND_DOWN;    gameTick(1);
                animState = STATE_REACH_FORWARD;gameTick(2);
                drawObject(OBJ_DRESSER_OPEN_1, DRESSER_X, DRESSER_Y);
                gameTick(2);
                drawObject(OBJ_DRESSER_CLOSED, DRESSER_X, DRESSER_Y);
                gameTick(2);
        }
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
