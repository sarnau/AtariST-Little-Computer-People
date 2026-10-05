/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* The resident turns to face the screen, waits for its head to swing
   round (waitHeadTurn), bends down for a moment and straightens up again --
   a "stand and look" gesture.  playRecord and stopRecord use this copy; tvOn
   and tvOff use the otherwise identical tvStoop.  Changes resFacing,
   animState and the head target headTarget. */
void
recordStoop()
{
        /* Three unused locals, but they must stay: removing them
           changes the compiled code.  The 1985 source carried two
           copies of this gesture (see tvStoop) with different
           declarations. */
        short   unused1;
        short   unused2;
        short   unused3;

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
