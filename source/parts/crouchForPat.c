/* The resident walks to the red armchair beside the phone on the
   ground floor, turns side-on and crouches down, then sets patAllowed
   so the player's Ctrl-P pat is accepted.  Gives up without crouching
   if the walk is preempted by a new action.  Called for ACTION_CROUCH_FOR_PAT
   and on the way into waitForPat, readInArmchair and answering the phone. */
void
crouchForPat()
{
        /* The walk call is tested in place, with no local; adding
           one would change the compiled code. */

        posToXY(POS_BTM_ARMCHAIR, &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;
        animState = STATE_STAND_SIDE_VIEW;
        resFacing = FACING_RIGHT;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();
        animState = STATE_CROUCH_DOWN;
        gameTick(5);
        patAllowed = YES;
}
