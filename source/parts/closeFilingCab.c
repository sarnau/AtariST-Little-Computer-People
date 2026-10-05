/* closeFilingCab: close the filing cabinet.  Assumes the resident is already
   standing at it: he bends, reaches and picks up, the drawer is drawn
   half then fully shut, and filingCabOpen is cleared.  Plays no sound. */
void
closeFilingCab()
{
        animState = STATE_BEND_DOWN;         gameTick(1);
        animState = STATE_REACH_FORWARD;     gameTick(2);
        animState = STATE_PICK_UP_FROM_FLOOR;gameTick(2);
        animState = STATE_REACH_FORWARD;
        drawObject(OBJ_FILING_CAB_OPEN_1, FILING_CAB_X, FILING_CAB_Y);
        gameTick(1);
        animState = STATE_BEND_DOWN;
        drawObject(OBJ_FILING_CABINET_CLOSED, FILING_CAB_X, FILING_CAB_Y);
        gameTick(1);
        filingCabOpen = NO;
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
