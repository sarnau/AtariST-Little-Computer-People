/*
 * Same gesture as recordStoop; the 1985 source carries two copies.
 * Included by stx_u2.c; never compiled on its own.
 */

void
tvStoop()
{
        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();
        animState = STATE_BEND_DOWN;
        gameTick(4);
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
