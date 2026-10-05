/*
 * The eat routine: takes one item of food from the cabinet, eats
 * 10..20 bite/chew cycles at the table, and resets hunger at the end.
 */


void
eatFromCabinet()
{
        short   food_count;
        short   inner;
        short   eat_cycles;
        short   saved_head_frame;

        scratchArr[0] = STATE_EAT_BITE;
        scratchArr[1] = STATE_EAT_CHEW;
        noPreempt = YES;

        posToXY(POS_BTM_KITCHEN_CABINET,
                              &walkXTarget, &walkYTarget);
        walkToTarget();

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();

        openKitchenCab(DOOR_OPEN);

        food_count = (resident.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD;
        if (food_count == 0) {
                gameTick(2);
                return;
        }

        animState = STATE_REACH_INTO_CABINET;
        gameTick(3);
        food_count--;
        resident.door_states_and_flags =
                (food_count << DSF_FOOD_SHIFT) |
                (resident.door_states_and_flags & ~DSF_FOOD_MASK);
        drawFoodCab();
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(2);

        if (resident.initiative_threshold < rndRng(0, 100))
                openKitchenCab(DOOR_CLOSE);

        carryBehind(SPRITE_FOOD_PACKAGE);
        posToXY(POS_BTM_KITCHEN_CABINET,
                              &walkXTarget, &walkYTarget);
        walkToTarget();
        posToXY(POS_BTM_KITCHEN_SINK,
                              &walkXTarget, &walkYTarget);
        walkToTarget();

        spriteLayer[SPRITE_TABLE_SETTING] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_TABLE_SETTING);
        pendX[spriteSlot[SPRITE_TABLE_SETTING]] = 103;
        pendY[spriteSlot[SPRITE_TABLE_SETTING]] = 180;

        posToXY(POS_BTM_TABLE_RIGHT,
                              &walkXTarget, &walkYTarget);
        walkToTarget();
        posToXY(POS_BTM_TABLE_LEFT,
                              &walkXTarget, &walkYTarget);
        walkToTarget();

        headMode       = HEAD_ANIM_DISABLED;
        animState            = STATE_STAND_SIDE_VIEW;
        resFacing = FACING_RIGHT;
        carryInFront(SPRITE_FOOD_PACKAGE);
        headTarget = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        waitHeadTurn();

        animState        = scratchArr[0];
        resY += 8;
        resX += 6;
        saved_head_frame = headFrame;
        eat_cycles       = rndRng(10, 20);
        headTarget = HEAD_ANIM_DISABLED;
        headPose      = HEAD_ANIM_DISABLED;
        gameTick(0);
        isCarrying = NO;
        pendX[spriteSlot[SPRITE_FOOD_PACKAGE]] += 3;
        pendY[spriteSlot[SPRITE_FOOD_PACKAGE]] -= 4;
        gameTick(0);

        while (eat_cycles-- > 0) {
                animState = scratchArr[1];
                gameTick(2);
                headFrame = 0;
                gameTick(rndRng(1, 2));
                headFrame = saved_head_frame;
                animState = scratchArr[0];
                gameTick(0);

                inner = rndRng(4, 8);
                while (inner-- > 0) {
                        if (eventQueue[0] != ACTION_NONE)
                                break;
                        headFrame = saved_head_frame;
                        gameTick(rndRng(1, 2));
                        headFrame = 1;
                        gameTick(0);
                        headFrame = 2;
                        gameTick(0);
                }
                headFrame = saved_head_frame;
        }

        isCarrying = YES;
        headTarget   = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        headPose        = HEAD_POSE(HEAD_DIR_FRONT, HEAD_TILT_LOWER);
        carryBehind(SPRITE_FOOD_PACKAGE);
        resY -= 8;
        resX -= 6;
        animState = STATE_STAND_SIDE_VIEW;
        waitHeadTurn();
        gameTick(0);

        posToXY(POS_BTM_TABLE_RIGHT,
                              &walkXTarget, &walkYTarget);
        walkToTarget();
        posToXY(POS_BTM_KITCHEN_SINK,
                              &walkXTarget, &walkYTarget);
        walkToTarget();

        spriteLayer[SPRITE_TABLE_SETTING] = SPRITE_HIDDEN;
        layoutSlots();
        spriteLayer[SPRITE_FOOD_PACKAGE]  = SPRITE_HIDDEN;
        layoutSlots();
        isCarrying = NO;

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        gameTick(4);

        resident.hunger_level   = NEED_SATISFIED;
        resident.bathroom_timer = resident.bathroom_timer_max;
        startRecovery();
        noPreempt = NO;
}
