/* A record delivery: fetch it from the front step and take it
   upstairs. */
void
recordDelivery()
{
        short   unused;         /* never written, but must stay */

        noPreempt = YES;
        walkToFrontDoor();
        /* The pick-up sequence is written out in each delivery
           handler, not factored into a helper. */
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        openFrontDoor(DOOR_OPEN);

        animState = STATE_BEND_DOWN;
        gameTick(1);
        animState = STATE_REACH_FORWARD;
        gameTick(2);
        animState = STATE_BEND_DOWN;
        gameTick(1);
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);

        if (resident.initiativeThreshold < rndRng(0, 100))
                openFrontDoor(DOOR_CLOSE);

        carryBehind(SPRITE_VINYL_CARRY);
        posToXY(POS_TOP_DANCE_FLOOR, &walkXTarget, &walkYTarget);
        walkToTarget();

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        spriteLayer[SPRITE_VINYL_CARRY] = SPRITE_HIDDEN;
        layoutSlots();
        isCarrying = NO;
        waitHeadTurn();

        animState = STATE_BEND_DOWN;    gameTick(1);
        animState = STATE_REACH_FORWARD; gameTick(2);
        animState = STATE_BEND_DOWN;    gameTick(1);
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);

        foodSupply++;                 /* 1985 typo, kept on purpose */
        noPreempt = NO;
}
