/* The resident dozes off.  With SLEEP_RANDOM (ACTION_SLEEP, moveInScene,
   and gameLoop's endless loop when the copy protection fails) he first
   walks to the centre line of the floor he is on and turns side-on,
   then sleeps 7..15 rounds; any other value is the round count, slept
   on the spot (playGame's dozeOff(1) between key polls).  Each round
   plays the breathe-in/out poses from scratchArr and SFX_SNORING.  Does
   nothing while he is on the stairs (onStairs); a queued action
   (eventQueue[0]) wakes him early. */
void
dozeOff(value)
short   value;
{
        short   i;
        short   duration;

        scratchArr[0] = STATE_SLP_BREATHE_I;
        scratchArr[1] = STATE_SLP_BREATHE_O;

        if (onStairs != NO)
                return;

        if (value == SLEEP_RANDOM) {
                walkXTarget = resX;
                walkYTarget = floorWalkY[floorOfY(resY) - 1];
                if (walkToTarget() != 0)
                        return;
                resFacing   = FACING_RIGHT;
                animState              = STATE_STAND_SIDE_VIEW;
                headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
                waitHeadTurn();
        }

        duration = rndRng(7, 15);
        if (value != SLEEP_RANDOM)
                duration = value;

        i = 0;
        while (i < duration) {
                if (eventQueue[0] != ACTION_NONE)
                        break;
                animState = scratchArr[0]; gameTick(1);
                animState = scratchArr[1]; gameTick(0);
                sfxSelect(SFX_SNORING, 3L);
                gameTick(1);
                animState = scratchArr[0]; gameTick(1);
                i++;
        }

        if (value == SLEEP_RANDOM) {
                animState = STATE_STAND_SIDE_VIEW;
                gameTick(0);
        }
}
