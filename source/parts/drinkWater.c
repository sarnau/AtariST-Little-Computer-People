/* drinkWater: get a drink of water.  The resident walks to the kitchen
   sink, picks up the glass and carries it to the water tap.  If the
   water tank (waterLevel) is not empty he bends, draws 3 units
   (updateWaterTank(-3)), drinks for 16 ticks and rinses the glass at the sink
   (washAtSink).  Thirst is reset to satisfied with a full timer whether or
   not there was water, and startRecovery may start recovery from sickness. */
void
drinkWater()
{
        posToXY(POS_BTM_KITCHEN_SINK, &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        noPreempt = YES;
        carryBehind(SPRITE_GLASS);
        posToXY(POS_BTM_WATER_TAP, &walkXTarget, &walkYTarget);
        walkToTarget();

        spriteLayer[SPRITE_GLASS] = SPRITE_HIDDEN;
        layoutSlots();
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        if (waterLevel != 0) {
                animState = STATE_BEND_DOWN;
                resFacing = FACING_RIGHT;
                gameTick(0);
                updateWaterTank(-3);
                headMode = HEAD_ANIM_DISABLED;
                animState = STATE_DRINK_FROM_GLASS;
                gameTick(16);
                animState = STATE_STAND_FACING_SCREEN;
                resY++;
                gameTick(3);
                washAtSink(3);
        }

        resident.thirstLevel = NEED_SATISFIED;
        resident.thirstTimer = resident.thirstTimerMax;
        startRecovery();
        spriteLayer[SPRITE_GLASS] = SPRITE_HIDDEN;
        layoutSlots();
        isCarrying = NO;
        noPreempt = NO;
}
