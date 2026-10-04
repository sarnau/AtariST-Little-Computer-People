/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* a_opecd: open (DOOR_OPEN, 0) or close (DOOR_CLOSE) a dresser drawer
   while the resident stands at it, updating lcp_drsO.  He bends and
   reaches while the drawer is drawn half then fully open or shut.
   Plays no sound.  A request matching the current state returns
   immediately. */
void
a_opecd(oc_stat)
short   oc_stat;
{
        if (oc_stat == 0) {
                if (lcp_drsO != NO)
                        return;
                lcp_drsO = YES;
                lcp_st = STATE_BEND_DOWN;    gameTick(1);
                lcp_st = STATE_REACH_FORWARD;gameTick(2);
                od_draw(OBJ_DRESSER_OPEN_1, DRESSER_X, DRESSER_Y);
                gameTick(2);
                od_draw(OBJ_DRESSER_OPEN_2, DRESSER_X, DRESSER_Y);
                gameTick(2);
        } else if (oc_stat != 0) {      /* redundant re-test, kept on purpose */
                if (lcp_drsO == NO)
                        return;
                lcp_drsO = NO;
                lcp_st = STATE_BEND_DOWN;    gameTick(1);
                lcp_st = STATE_REACH_FORWARD;gameTick(2);
                od_draw(OBJ_DRESSER_OPEN_1, DRESSER_X, DRESSER_Y);
                gameTick(2);
                od_draw(OBJ_DRESSER_CLOSED, DRESSER_X, DRESSER_Y);
                gameTick(2);
        }
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
