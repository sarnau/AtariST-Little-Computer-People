/*
 * parts/foodDelivery.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */


/* Food delivery.  The resident walks to the front door, opens it and
   picks up the package (SPRITE_FOOD_PACKAGE, carried), closing the
   door when a 0..100 roll beats his initiative_threshold.  For dog
   food (g_dvdog) he fills the bowl if it is empty (feedDog) or else
   takes the package to the fridge (goToFridge).  Otherwise he stocks the
   kitchen cabinet: one reach per pack until the food-count field in
   lcp.door_states_and_flags reaches FOOD_PACKS_MAX, redrawing the
   cabinet's food markers (drawFoodCab) each time. */
void
foodDelivery()
{
        short   food_count;
        short   roll;           /* unused, but it must stay: removing it changes the compiled code */

        g_actif = YES;
        walkToFrontDoor();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        waitHeadTurn();
        openFrontDoor(DOOR_OPEN);

        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_REACH_FORWARD;
        gameTick(2);
        lcp_st = STATE_BEND_DOWN;
        gameTick(1);
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);

        if (lcp.initiative_threshold < rndRng(0, 100))
                openFrontDoor(DOOR_CLOSE);

        if (g_dvdog != NO) {
                carryBehind(SPRITE_FOOD_PACKAGE);
                if (lcp_bwlS == BOWL_EMPTY) {
                        feedDog(1);
                } else {
                        goToFridge();
                        g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
                        layoutSlots();
                        g_lcyof = NO;
                }
        } else {
                carryBehind(SPRITE_FOOD_PACKAGE);
                posToXY(POS_BTM_KITCHEN_CABINET,
                                      &g_wtx, &g_wty);
                walkToTarget();

                g_selaf[SPRITE_FOOD_PACKAGE] = SPRITE_HIDDEN;
                layoutSlots();
                g_lcyof = NO;
                lcp_face     = FACING_RIGHT;
                lcp_st                = STATE_STAND_FACING_SCREEN;
                g_hatas   = HEAD_ANIM_HORIZONTAL_RANGE;
                waitHeadTurn();

                openKitchenCab(DOOR_OPEN);

                /* The flag is tested a second time -- redundant inside
                   this arm, but that is what the original does. */
                if (g_dvdog == NO) {
                        while (1) {
                                food_count =
                                        (lcp.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD;
                                food_count++;
                                if (food_count > FOOD_PACKS_MAX)
                                        break;
                                food_count = food_count << DSF_FOOD_SHIFT;
                                lcp.door_states_and_flags &= ~DSF_FOOD_MASK;
                                lcp.door_states_and_flags |= food_count;
                                lcp_st = STATE_REACH_INTO_CABINET;
                                gameTick(3);
                                drawFoodCab();
                                lcp_st = STATE_STAND_FACING_SCREEN;
                                gameTick(1);
                        }
                }

                if (lcp.initiative_threshold < rndRng(0, 100))
                        openKitchenCab(DOOR_CLOSE);
                g_actif = NO;
        }
}
