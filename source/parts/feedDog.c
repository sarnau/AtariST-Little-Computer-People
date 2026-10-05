/* feedDog: fill the dog's bowl.  With value 0 the resident first
   fetches a food package from the fridge (door opens, he reaches in);
   with value non-zero the caller (a food delivery) has already put the
   package in his hands.  He carries it to the dog bowl, bends and
   fills it (bowlLevel = BOWL_FULL, bowlChange flags the change for the
   tick loop), then carries the package back and stores it (putInFridge). */
void
feedDog(value)
short   value;
{
        /* The call is tested in place, with no local. */

        if (value == 0) {
                posToXY(POS_BTM_FRIDGE, &walkXTarget, &walkYTarget);
                if (walkToTarget() != 0)
                        return;

                resFacing = FACING_RIGHT;
                animState = STATE_STAND_FACING_SCREEN;
                headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
                waitHeadTurn();

                resFacing = FACING_LEFT;
                animState = STATE_REACH_INTO_CABINET;
                drawObject(OBJ_FRIDGE_CLOSED, FRIDGE_X, FRIDGE_Y);
                gameTick(1);
                drawObject(OBJ_FRIDGE_OPEN_1, FRIDGE_X, FRIDGE_Y);
                sfxSelect(SFX_DOOR_OPEN, 6L);
                gameTick(1);
                drawObject(OBJ_FRIDGE_OPEN_2, FRIDGE_X, FRIDGE_Y);
                gameTick(1);

                resFacing = FACING_RIGHT;
                animState = STATE_STAND_FACING_SCREEN;
                gameTick(2);

                resFacing = FACING_LEFT;
                animState = STATE_REACH_INTO_CABINET;
                gameTick(3);

                resFacing = FACING_RIGHT;
                animState = STATE_STAND_FACING_SCREEN;
                gameTick(2);

                drawObject(OBJ_FRIDGE_OPEN_1, FRIDGE_X, FRIDGE_Y);
                gameTick(1);
                drawObject(OBJ_FRIDGE_CLOSED, FRIDGE_X, FRIDGE_Y);
                sfxSelect(SFX_DOOR_OPEN, 6L);
                gameTick(1);

                carryBehind(SPRITE_FOOD_PACKAGE);
        }

        posToXY(POS_BTM_DOG_BOWL, &walkXTarget, &walkYTarget);
        noPreempt = YES;
        walkToTarget();

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        spriteLayer[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
        layoutSlots();
        isCarrying = NO;
        waitHeadTurn();

        animState = STATE_BEND_DOWN;    gameTick(1);
        animState = STATE_REACH_FORWARD;gameTick(2);
        animState = STATE_BEND_DOWN;    gameTick(1);

        bowlChange = 1;
        bowlLevel = BOWL_FULL;
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);

        carryBehind(SPRITE_FOOD_PACKAGE);
        posToXY(POS_BTM_FRIDGE, &walkXTarget, &walkYTarget);
        noPreempt = YES;
        walkToTarget();

        spriteLayer[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
        layoutSlots();
        isCarrying = NO;
        putInFridge();
        noPreempt = NO;
}
