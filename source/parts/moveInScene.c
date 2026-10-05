/* The new-resident move-in cutscene.  The screen is empty
   while the delivery van pulls up (two playDoorbell door-bell blasts), the
   front door opens, the dog is placed on the step, then the resident
   walks in and does a full tour of the house -- dresser, sink, food,
   TV, bed -- before the dog is released and the intro flag drops. */

void
moveInScene()
{
        short   unused;         /* never referenced, but must stay */

        dogHidden = 1;
        movingIn = 1;
        hideResident();
        gameTick(240);
        playDoorbell();
        gameTick(80);
        playDoorbell();
        gameTick(24);

        /* Front door swings open behind the doorbell sound. */
        drawObject(OBJ_DOOR_FRONT_OPEN_1, FRONT_DOOR_X, FRONT_DOOR_Y);
        sfxSelect(SFX_DOOR_OPEN, 6L);
        gameTick(2);
        drawObject(OBJ_DOOR_FRONT_OPEN_2, FRONT_DOOR_X, FRONT_DOOR_Y);
        gameTick(2);
        frontDoorOpen = 1;

        /* The dog is waiting on the step. */
        spriteLayer[SPRITE_DOG_SIT] = 1;
        activateSprite(SPRITE_DOG_SIT);
        pendX[spriteSlot[SPRITE_DOG_SIT]] = 294;
        pendY[spriteSlot[SPRITE_DOG_SIT]] = 151;

        resX = 300;
        resY = 190;
        showResident();
        posToXY(POS_BTM_SCREEN_EDGE, &walkXTarget, &walkYTarget);
        walkXTarget -= 50;
        walkToTarget();
        animState  = STATE_STAND_SIDE_VIEW;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();
        spriteLayer[SPRITE_DOG_SIT] = 0;
        layoutSlots();
        gameTick(16);

        /* The protection result gates the game: a failed check parks
           the resident asleep for ever. */
        if (copyProtResult == 0)
                while (1)
                        dozeOff(SLEEP_RANDOM);

        posToXY(POS_BTM_KITCHEN_CABINET, &walkXTarget, &walkYTarget);
        walkToTarget();
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        openKitchenCab(DOOR_OPEN);
        gameTick(16);
        openKitchenCab(DOOR_CLOSE);

        posToXY(POS_BTM_KITCHEN_SINK, &walkXTarget, &walkYTarget);
        walkToTarget();
        gameTick(8);
        goToFridge();
        tvOn();
        animState  = STATE_STAND_SIDE_VIEW;
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();
        nodOk();
        enterStudy(0);
        wakeFromAlarm();

        posToXY(POS_MID_DRESSER, &walkXTarget, &walkYTarget);
        walkToTarget();
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        changeClothes(0);
        useToilet();

        posToXY(POS_MID_BATHROOM_SINK, &walkXTarget, &walkYTarget);
        walkToTarget();
        goToFridge();
        useComputer();
        tidyHouse();
        idleShrug();
        tvOff();
        checkFrontDoor(100);
        walkToFrontDoor();
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        openFrontDoor(DOOR_OPEN);

        /* Bend down and pick the suitcase up off the step. */
        animState = STATE_BEND_DOWN;
        gameTick(1);
        animState = STATE_REACH_FORWARD;
        gameTick(2);
        animState = STATE_BEND_DOWN;
        gameTick(1);
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);
        carryBehind(SPRITE_SUITCASE);

        posToXY(POS_MID_DRESSER, &walkXTarget, &walkYTarget);
        walkToTarget();
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        spriteLayer[SPRITE_SUITCASE] = 0;
        layoutSlots();
        isCarrying = 0;
        waitHeadTurn();
        openDresser(DOOR_OPEN);

        /* Let the dog in and seed its first wander target. */
        posToXY(POS_BTM_FRONT_DOOR, &dogX, &dogY);
        dogY = 190;
        dogX = 273;
        posToXY(dogLastPick = dogStartPos, &dogXTarget, &dogYTarget);
        dogYTarget += dogYStartNudge;
        dogXWaypt = dogXTarget;
        dogYWaypt = dogYTarget;
        dogOnStairs = 0;
        dogIdleCount = 20;
        dogHidden = 0;
        setDogSprite(SPRITE_DOG_LAY_DOWN, -1, 1);

        changeClothes(0);
        enterStudy(1);
        movingIn = 0;
}
