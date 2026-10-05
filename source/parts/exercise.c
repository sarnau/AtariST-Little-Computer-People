/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* exercise: exercise.  The resident walks to the middle-floor couch
   and, facing side-on, cycles through the four arm-exercise poses
   (centre, up, centre, wide) for 8..127 steps, three ticks on each
   outstretched pose and one on centre, stopping early when a new
   action is queued. */
void
exercise()
{
        short           result;
        /* Signed shorts, declared in this order, as in the original.
           `result` holds the per-frame tick count, set to 3 before the
           loop and passed to gameTick. */
        short           duration;
        short           i;

        scratchArr[0] = STATE_EX_ARMS_CTR;
        scratchArr[1] = STATE_EX_ARMS_UP;
        scratchArr[2] = STATE_EX_ARMS_CTR;
        scratchArr[3] = STATE_EX_ARMS_WIDE;

        posToXY(POS_MID_COUCH,
                              &walkXTarget, &walkYTarget);
        /* `-=` and the inline test of the walk call are part of the
           original code. */
        walkYTarget -= 5;
        if (walkToTarget() != 0)
                return;

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_SIDE_VIEW;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();

        /* The mask is folded into the assignment (computed once) and
           the counter steps with i++, as in the original. */
        result = 3;
        duration = (unsigned short)(Random() & 0x7f) | 8;
        i = 0;
        while (i < duration) {
                if (eventQueue[0] != ACTION_NONE)
                        break;
                animState = scratchArr[i & 3];
                if (animState == STATE_EX_ARMS_CTR)
                        gameTick(0);
                else
                        gameTick(result);
                i++;
        }
        animState = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}
