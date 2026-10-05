/*
 * parts/foodDelivery.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */


/* Food delivery.  The resident walks to the front door, opens it and
   picks up the package (SPRITE_FOOD_PACKAGE, carried), closing the
   door when a 0..100 roll beats his initiative_threshold.  For dog
   food (isDogDelivery) he fills the bowl if it is empty (feedDog) or else
   takes the package to the fridge (goToFridge).  Otherwise he stocks the
   kitchen cabinet: one reach per pack until the food-count field in
   resident.door_states_and_flags reaches FOOD_PACKS_MAX, redrawing the
   cabinet's food markers (drawFoodCab) each time. */
void
foodDelivery()
{
        short   food_count;
        short   roll;           /* unused, but it must stay: removing it changes the compiled code */

        noPreempt = YES;
        walkToFrontDoor();

        resFacing   = FACING_RIGHT;
        animState              = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_ANIM_HORIZONTAL_RANGE;
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

        if (isDogDelivery != NO) {
                carryBehind(SPRITE_FOOD_PACKAGE);
                if (bowlLevel == BOWL_EMPTY) {
                        feedDog(1);
                } else {
                        goToFridge();
                        spriteLayer[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
                        layoutSlots();
                        isCarrying = NO;
                }
        } else {
                carryBehind(SPRITE_FOOD_PACKAGE);
                posToXY(POS_BTM_KITCHEN_CABINET,
                                      &walkXTarget, &walkYTarget);
                walkToTarget();

                spriteLayer[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
                layoutSlots();
                isCarrying = NO;
                resFacing     = FACING_RIGHT;
                animState                = STATE_STAND_FACING_SCREEN;
                headTarget   = HEAD_ANIM_HORIZONTAL_RANGE;
                waitHeadTurn();

                openKitchenCab(DOOR_OPEN);

                /* The flag is tested a second time -- redundant inside
                   this arm, but that is what the original does. */
                if (isDogDelivery == NO) {
                        while (1) {
                                food_count =
                                        (resident.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD;
                                food_count++;
                                if (food_count > FOOD_PACKS_MAX)
                                        break;
                                food_count = food_count << DSF_FOOD_SHIFT;
                                resident.door_states_and_flags &= ~DSF_FOOD_MASK;
                                resident.door_states_and_flags |= food_count;
                                animState = STATE_REACH_INTO_CABINET;
                                gameTick(3);
                                drawFoodCab();
                                animState = STATE_STAND_FACING_SCREEN;
                                gameTick(1);
                        }
                }

                if (resident.initiative_threshold < rndRng(0, 100))
                        openKitchenCab(DOOR_CLOSE);
                noPreempt = NO;
        }
}
