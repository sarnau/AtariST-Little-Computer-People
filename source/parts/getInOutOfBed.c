/* Get into or out of bed, toggling resident.isSleeping.  Awake:
   the resident walks to the bed and, unless preempted, undresses,
   gets in and lies down, stepping left as each pose plays.  Asleep:
   the same poses play in reverse, stepping right, and he ends
   standing by the bed.  runAction calls it first whenever an action
   arrives while he is asleep. */
void
getInOutOfBed()
{
        /* The call is tested in place, with no local. */

        scratchArr[0] = STATE_UNDRESS_AT_BED;
        scratchArr[1] = STATE_LIE_DOWN_GETTING_IN;
        scratchArr[2] = STATE_LIE_DOWN_IN_BED;

        if (resident.isSleeping == NO) {
                posToXY(POS_MID_BED, &walkXTarget, &walkYTarget);
                if (walkToTarget() != 0)
                        return;
                resFacing = FACING_RIGHT;
                animState = STATE_STAND_IDLE;
                headTarget = HEAD_POSE(HEAD_DIR_RIGHT, HEAD_TILT_LOWER);
                waitHeadTurn();
                resident.isSleeping = YES;
                resX -= 10;
                resFacing = FACING_RIGHT;
                animState = scratchArr[0]; gameTick(2);
                resX -= 8;
                animState = scratchArr[1]; gameTick(2);
                resX -= 2;
                animState = scratchArr[2]; gameTick(2);
        } else {
                resFacing = FACING_RIGHT;
                resX += 10;
                animState = scratchArr[1]; gameTick(2);
                resX += 10;
                animState = scratchArr[0]; gameTick(2);
                resident.isSleeping = NO;
                animState = STATE_STAND_IDLE;
                headTarget = HEAD_POSE(HEAD_DIR_RIGHT, HEAD_TILT_LOWER);
                waitHeadTurn();
                gameTick(2);
        }
}
