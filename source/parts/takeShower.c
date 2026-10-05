/*
 * parts/takeShower.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* ACTION_TAKE_SHOWER (also nightRoutine and morningRoutine).  The resident walks
   to the shower door -- giving up if interrupted there -- then, now
   committed (noPreempt), steps into the stall and showers for 20..25
   rounds, each a random choice of washing or scrubbing left/right,
   with the head on HEAD_ANIM_SHOWER.  Afterwards he steps down out of
   the stall, walks back to the door and the head animation and
   noPreempt are reset. */
void
takeShower()
{
        /* walkToTarget()'s result is tested in place, with no local for it. */
        short   count;

        posToXY(POS_MID_SHOWER_DOOR,
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        posToXY(POS_MID_SHOWER_INSIDE,
                              &walkXTarget, &walkYTarget);
        noPreempt = YES;
        walkToTarget();

        resFacing = FACING_RIGHT;
        animState = STATE_SHOWER_STAND;
        resX -= 8;
        resY -= 23;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();
        headMode = HEAD_ANIM_SHOWER;

        count = rndRng(20, 25);
        while (count-- != 0) {          /* post-decrement test, on purpose */
                /* The call is tested in place; the arm order is the
                   original's and affects the compiled code. */
                if (rndRng(0, 1) != 0) {
                        animState = STATE_SHR_WASH_L;   gameTick(2);
                        animState = STATE_SHR_WASH_R;  gameTick(2);
                        animState = STATE_SHR_WASH_L;   gameTick(2);
                        animState = STATE_SHR_WASH_R;  gameTick(2);
                        animState = STATE_SHOWER_STAND;       gameTick(4);
                } else {
                        animState = STATE_SHR_SCRUB_L;  gameTick(2);
                        animState = STATE_SHR_SCRUB_R; gameTick(2);
                        animState = STATE_SHR_SCRUB_L;  gameTick(2);
                        animState = STATE_SHR_SCRUB_R; gameTick(2);
                        animState = STATE_SHOWER_STAND;       gameTick(4);
                }
        }

        animState = STATE_STAND_FACING_SCREEN;
        resY += 29;
        gameTick(2);
        posToXY(POS_MID_SHOWER_DOOR,
                              &walkXTarget, &walkYTarget);
        walkToTarget();
        headMode = HEAD_ANIM_DISABLED;
        noPreempt = NO;
}
