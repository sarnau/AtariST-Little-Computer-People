/*
 * parts/yawnAndStretch.c -- included by stx_u2.c; never compiled on its own.
 */

/* ACTION_YAWN_AND_STRETCH: the resident turns side-on and, for 15
   ticks, alternates the open-mouthed yawn and the arm stretch, then
   stands side-on again. */
void
yawnAndStretch()
{
        short   i;

        scratchArr[0]  = STATE_YAWN_MOUTH_OPEN;
        scratchArr[1]  = STATE_YAWN_STRETCH_ARMS;
        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_SIDE_VIEW;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();

        /* Written as i++ on purpose: `i = i + 1` compiles differently. */
        for (i = 0; i < 15; i++) {
                animState = scratchArr[i & 1];
                gameTick(1);
        }
        animState = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
