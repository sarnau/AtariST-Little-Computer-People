/*
 * parts/rummageCabinet.c -- included by stx_u2.c; never compiled on its own.
 */

/* rummageCabinet: rummage in the filing cabinet.  The resident bends down,
   opens the drawer if it is shut (lcp_flcO, drawn in two frames), then
   holds the reaching pose (the state is named STOKE_FIREPLACE, but here
   it is used at the cabinet) while glancing left and right ten times.
   Shared by tidying the house, fetching letter paper and fetching the
   game box -- all of which live in that cabinet. */

void
rummageCabinet()
{
        short   i;

        lcp_st = STATE_BEND_DOWN;
        gameTick(1);

        if (lcp_flcO == NO) {
                lcp_flcO = YES;
                lcp_st = STATE_REACH_FORWARD;
                drawObject(OBJ_FILING_CAB_OPEN_1, FILING_CAB_X, FILING_CAB_Y);
                gameTick(2);
                lcp_st = STATE_PICK_UP_FROM_FLOOR;
                drawObject(OBJ_FILING_CAB_OPEN_2, FILING_CAB_X, FILING_CAB_Y);
                gameTick(2);
        } else {
                lcp_st = STATE_REACH_FORWARD;
                gameTick(1);
        }

        lcp_st = STATE_STOKE_FIREPLACE;
        gameTick(1);
        /* Written as i++ on purpose: i = i + 1 compiles differently. */
        for (i = 0; i < 10; i++) {
                lcp_face = rndRng(0, 1);
                gameTick(0);
        }

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
