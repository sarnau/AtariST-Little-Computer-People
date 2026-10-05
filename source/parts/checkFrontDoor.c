/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* checkFrontDoor: check the front door.  The resident walks to the front
   door, opens it if shut, steps outside (the sitting-dog sprite waits
   on the porch, the resident is hidden) for `value` ticks, comes back
   in and the dog sprite is removed.  If a random roll beats
   resident.initiative_threshold he walks back and shuts the door again.
   noPreempt keeps the inner walks from being preempted. */
void
checkFrontDoor(value)
short   value;
{

        posToXY(POS_BTM_FRONT_DOOR,
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        if (frontDoorOpen == NO)
                openFrontDoor(DOOR_OPEN);
        noPreempt = YES;

        posToXY(POS_BTM_FRONT_DOOR,
                              &walkXTarget, &walkYTarget);
        walkXTarget -= 10;
        walkToTarget();

        spriteLayer[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOG_SIT);
        pendX[spriteSlot[SPRITE_DOG_SIT]] = 294;
        pendY[spriteSlot[SPRITE_DOG_SIT]] = 151;

        posToXY(POS_BTM_FRONT_DOOR,
                              &walkXTarget, &walkYTarget);
        walkToTarget();
        hideResident();
        gameTick(value);
        showResident();

        posToXY(POS_BTM_FRONT_DOOR,
                              &walkXTarget, &walkYTarget);
        walkXTarget -= 10;
        walkToTarget();
        spriteLayer[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        layoutSlots();

        if (resident.initiative_threshold < rndRng(0, 100)) {
                noPreempt = YES;
                posToXY(POS_BTM_FRONT_DOOR,
                                      &walkXTarget, &walkYTarget);
                walkToTarget();
                resFacing   = FACING_RIGHT;
                animState              = STATE_STAND_FACING_SCREEN;
                headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
                waitHeadTurn();
                openFrontDoor(DOOR_CLOSE);
        }
        noPreempt = NO;
}
