/* rummageCabinet: rummage in the filing cabinet.  The resident bends down,
   opens the drawer if it is shut (filingCabOpen, drawn in two frames), then
   holds the reaching pose (the state is named STOKE_FIREPLACE, but here
   it is used at the cabinet) while glancing left and right ten times.
   Shared by tidying the house, fetching letter paper and fetching the
   game box -- all of which live in that cabinet. */

void
rummageCabinet()
{
        short   i;

        animState = STATE_BEND_DOWN;
        gameTick(1);

        if (filingCabOpen == NO) {
                filingCabOpen = YES;
                animState = STATE_REACH_FORWARD;
                drawObject(OBJ_FILING_CAB_OPEN_1, FILING_CAB_X, FILING_CAB_Y);
                gameTick(2);
                animState = STATE_PICK_UP_FROM_FLOOR;
                drawObject(OBJ_FILING_CAB_OPEN_2, FILING_CAB_X, FILING_CAB_Y);
                gameTick(2);
        } else {
                animState = STATE_REACH_FORWARD;
                gameTick(1);
        }

        animState = STATE_STOKE_FIREPLACE;
        gameTick(1);
        /* Written as i++ on purpose: i = i + 1 compiles differently. */
        for (i = 0; i < 10; i++) {
                resFacing = rndRng(0, 1);
                gameTick(0);
        }

        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
