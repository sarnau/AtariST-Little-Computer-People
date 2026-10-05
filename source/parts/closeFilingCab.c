/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* closeFilingCab: close the filing cabinet.  Assumes the resident is already
   standing at it: he bends, reaches and picks up, the drawer is drawn
   half then fully shut, and lcp_flcO is cleared.  Plays no sound. */
void
closeFilingCab()
{
        lcp_st = STATE_BEND_DOWN;         gameTick(1);
        lcp_st = STATE_REACH_FORWARD;     gameTick(2);
        lcp_st = STATE_PICK_UP_FROM_FLOOR;gameTick(2);
        lcp_st = STATE_REACH_FORWARD;
        drawObject(OBJ_FILING_CAB_OPEN_1, FILING_CAB_X, FILING_CAB_Y);
        gameTick(1);
        lcp_st = STATE_BEND_DOWN;
        drawObject(OBJ_FILING_CABINET_CLOSED, FILING_CAB_X, FILING_CAB_Y);
        gameTick(1);
        lcp_flcO = NO;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
