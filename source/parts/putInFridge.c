/* putInFridge: put something back in the fridge.  Assumes the resident
   is already at it: he faces the screen, the fridge door is opened
   with the door sound, he reaches in, pauses, and the door is drawn
   shut again.  Both door movements play SFX_DOOR_OPEN.  Ends feedDog
   and the food-delivery path (goToFridge). */
void
putInFridge()
{
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        resFacing = FACING_LEFT;
        animState = STATE_REACH_INTO_CABINET;
        drawObject(OBJ_FRIDGE_CLOSED, FRIDGE_X, FRIDGE_Y);
        gameTick(1);
        drawObject(OBJ_FRIDGE_OPEN_1, FRIDGE_X, FRIDGE_Y);
        sfxSelect(SFX_DOOR_OPEN, 6L);
        gameTick(1);
        drawObject(OBJ_FRIDGE_OPEN_2, FRIDGE_X, FRIDGE_Y);
        gameTick(1);

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(2);

        resFacing = FACING_LEFT;
        animState = STATE_REACH_INTO_CABINET;
        gameTick(3);

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(8);

        drawObject(OBJ_FRIDGE_OPEN_1, FRIDGE_X, FRIDGE_Y);
        gameTick(1);
        drawObject(OBJ_FRIDGE_CLOSED, FRIDGE_X, FRIDGE_Y);
        sfxSelect(SFX_DOOR_OPEN, 6L);   /* OPEN, not CLOSE: as in the original */
        gameTick(1);
}
