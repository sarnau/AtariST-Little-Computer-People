/*
 * parts/openKitchenCab.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* openKitchenCab: open (DOOR_OPEN, 0) or close (DOOR_CLOSE) the kitchen
   cabinet while the resident stands at it, updating kitchenCabOpen.  Both
   directions reach in and step the door through its ajar frame with
   the matching sound; opening also draws the food-count markers on
   the shelves (drawFoodCab).  A request matching the current state
   returns immediately. */
void
openKitchenCab(oc_stat)
short   oc_stat;
{
        if (oc_stat == 0) {
                if (kitchenCabOpen != NO)
                        return;
                kitchenCabOpen = YES;
                animState = STATE_REACH_INTO_CABINET;
                gameTick(3);
                drawObject(OBJ_CABINET_OPEN_1, KITCHEN_CAB_X, KITCHEN_CAB_Y);
                sfxSelect(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                drawObject(OBJ_CABINET_OPEN_2, KITCHEN_CAB_X, KITCHEN_CAB_Y);
                drawFoodCab();
                animState = STATE_STAND_FACING_SCREEN;
                gameTick(2);
        } else if (oc_stat != 0) {      /* redundant re-test, kept on purpose */
                if (kitchenCabOpen == NO)
                        return;
                kitchenCabOpen = NO;
                animState = STATE_REACH_INTO_CABINET;
                gameTick(3);
                drawObject(OBJ_CABINET_OPEN_1, KITCHEN_CAB_X, KITCHEN_CAB_Y);
                gameTick(2);
                drawObject(OBJ_CABINET_CLOSED, KITCHEN_CAB_X, KITCHEN_CAB_Y);
                sfxSelect(SFX_DOOR_CLOSE, 6L);
                animState = STATE_STAND_FACING_SCREEN;
                gameTick(2);
        }
}
