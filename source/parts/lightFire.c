/*
 * parts/lightFire.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* lightFire: light the fireplace; does nothing if a fire is already
   burning.  The resident opens the front door, steps outside (the
   sitting-dog sprite waits on the porch) for 40 ticks, returns carrying
   firewood, perhaps shuts the door (random roll against
   resident.initiative_threshold), and walks to the fireplace, where he
   bends, stokes and fidgets for ten ticks.  Then fireBurning is set and
   fireTimeLeft is 2500..5000 ticks, which the tick loop counts down. */
void
lightFire()
{
        /* walkToTarget()'s result is tested in place, with no local for it. */
        short   i;

        if (fireBurning != NO)
                return;

        posToXY(POS_BTM_FRONT_DOOR,
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        openFrontDoor(DOOR_OPEN);
        noPreempt = YES;

        posToXY(POS_BTM_FRONT_DOOR,
                              &walkXTarget, &walkYTarget);
        walkXTarget -= 10;
        walkToTarget();

        /* Sit-dog sprite waits at the porch. */
        spriteLayer[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_DOG_SIT);
        pendX[spriteSlot[SPRITE_DOG_SIT]] = 294;
        pendY[spriteSlot[SPRITE_DOG_SIT]] = 151;

        posToXY(POS_BTM_FRONT_DOOR,
                              &walkXTarget, &walkYTarget);
        walkToTarget();
        hideResident();
        gameTick(40);
        showResident();

        carryBehind(SPRITE_FIREWOOD);
        posToXY(POS_BTM_FRONT_DOOR,
                              &walkXTarget, &walkYTarget);
        walkXTarget -= 10;
        walkToTarget();

        spriteLayer[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        layoutSlots();

        if (resident.initiative_threshold < rndRng(0, 100))
                openFrontDoor(DOOR_CLOSE);

        posToXY(POS_BTM_FIREPLACE_LOGS,
                              &walkXTarget, &walkYTarget);
        noPreempt = YES;
        walkToTarget();

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        spriteLayer[SPRITE_FIREWOOD] = SPRITE_HIDDEN;
        layoutSlots();
        isCarrying = NO;
        waitHeadTurn();

        resFacing = FACING_RIGHT;
        animState = STATE_BEND_DOWN;      gameTick(1);
        animState = STATE_REACH_FORWARD;  gameTick(1);
        animState = STATE_STOKE_FIREPLACE;gameTick(1);

        /* Random-direction shrug for 10 ticks (feeding kindling). */
        for (i = 0; i < 10; i++) {
                resFacing = rndRng(0, 1);
                gameTick(0);
        }

        fireBurning        = YES;
        fireTimeLeft = rndRng(2500, 5000);

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);
        noPreempt = NO;
}
