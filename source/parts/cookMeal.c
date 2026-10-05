/*
 * parts/cookMeal.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* cookMeal: cook and eat a meal.  The resident walks to the kitchen
   cabinet, takes out the cooking pot and carries it to the stove,
   where the pot sits on the hob while random flame frames flicker
   for 30..50 ticks; the stove is then drawn off and he carries the
   cooked meal back to the cabinet and eats it through eatFromCabinet. */
void
cookMeal()
{
        /* walkToTarget()'s result is tested in place, with no local for it. */
        short   counter;

        posToXY(POS_BTM_KITCHEN_CABINET,
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        animState = STATE_BEND_DOWN;    gameTick(1);
        animState = STATE_REACH_FORWARD;gameTick(2);
        animState = STATE_STAND_FACING_SCREEN; gameTick(0);

        carryBehind(SPRITE_COOKING_POT);
        posToXY(POS_BTM_STOVE,
                              &walkXTarget, &walkYTarget);
        noPreempt = YES;
        walkToTarget();

        spriteLayer[SPRITE_COOKING_POT] = SPRITE_HIDDEN;
        layoutSlots();
        carryBehind(SPRITE_COOKING_POT);
        isCarrying = NO;
        pendX[spriteSlot[SPRITE_COOKING_POT]] = 11;
        pendY[spriteSlot[SPRITE_COOKING_POT]] = 172;

        resFacing = FACING_LEFT;
        animState            = STATE_BEND_AND_REACH;

        /* 30..50 tick cooking animation, rotating stove frames. */
        counter = rndRng(30, 50);
        while (counter-- != 0) {
                drawObject(stoveFrames[rndRng(0, 2)], STOVE_X, STOVE_Y);
                gameTick(1);
        }
        drawObject(OBJ_STOVE_OFF, STOVE_X, STOVE_Y);

        spriteLayer[SPRITE_COOKING_POT] = SPRITE_HIDDEN;
        layoutSlots();
        carryBehind(SPRITE_COOKED_MEAL);

        /* Back to cabinet, then chain into eatFromCabinet to eat. */
        posToXY(POS_BTM_KITCHEN_CABINET,
                              &walkXTarget, &walkYTarget);
        noPreempt = YES;
        walkToTarget();
        spriteLayer[SPRITE_COOKED_MEAL] = SPRITE_HIDDEN;
        layoutSlots();
        isCarrying = NO;
        gameTick(0);
        eatFromCabinet();
        noPreempt = NO;
}
