/* Book delivery (ACTION_EVENT_BOOK_DELIVERY).  The resident walks to
   the front door uninterruptibly, opens it, bends down to pick up the
   parcel, and closes the door again when a 0..100 roll beats his
   initiative_threshold.  He then carries the book (SPRITE_BOOK) up to
   the bathroom entrance on the middle floor, the carried sprite is
   dropped (isCarrying cleared) and he reaches in to put it away. */
void
bookDelivery()
{
        noPreempt = YES;
        walkToFrontDoor();
        /* The pick-up sequence is written out in each handler rather
           than shared through a helper, as in the original. */
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

        if (resident.initiative_threshold < rndRng(0, 100))
                openFrontDoor(DOOR_CLOSE);

        carryBehind(SPRITE_BOOK);
        posToXY(POS_MID_BATHROOM_ENTRANCE, &walkXTarget, &walkYTarget);
        walkToTarget();

        spriteLayer[SPRITE_BOOK] = SPRITE_HIDDEN;
        layoutSlots();
        isCarrying = NO;
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        animState = STATE_REACH_INTO_CABINET;
        gameTick(3);
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(2);
        noPreempt = NO;
}
