/*
 * Same gesture as recordStoop; the 1985 source carries two copies.
 */

void
tvStoop()
{
        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        animState = STATE_BEND_DOWN;
        gameTick(4);
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
