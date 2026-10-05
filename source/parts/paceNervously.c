/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* paceNervously: pace nervously on the spot.  The resident turns side-on,
   waits for his head to settle, then alternates the shift-left and
   shift-right pacing poses for 15 ticks and stands still again. */
void
paceNervously()
{
        short   i;

        scratchArr[0]  = STATE_PACE_SHIFT_LEFT;
        scratchArr[1]  = STATE_PACE_SHIFT_RIGHT;
        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_SIDE_VIEW;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();

        /* Written i++ on purpose: i = i + 1 compiles differently. */
        for (i = 0; i < 15; i++) {
                animState = scratchArr[i & 1];
                gameTick(1);
        }
        animState = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
