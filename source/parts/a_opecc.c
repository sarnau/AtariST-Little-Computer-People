/*
 * parts/a_opecc.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* a_opecc: open (DOOR_OPEN, 0) or close (DOOR_CLOSE) the kitchen
   cabinet while the resident stands at it, updating lcp_cabO.  Both
   directions reach in and step the door through its ajar frame with
   the matching sound; opening also draws the food-count markers on
   the shelves (sc_drfc).  A request matching the current state
   returns immediately. */
void
a_opecc(oc_stat)
short   oc_stat;
{
        if (oc_stat == 0) {
                if (lcp_cabO != NO)
                        return;
                lcp_cabO = YES;
                lcp_st = STATE_REACH_INTO_CABINET;
                gameTick(3);
                od_draw(od_cbo1, 46, 140);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(od_cbo2, 46, 140);
                sc_drfc();
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(2);
        } else if (oc_stat != 0) {      /* redundant re-test, kept on purpose */
                if (lcp_cabO == NO)
                        return;
                lcp_cabO = NO;
                lcp_st = STATE_REACH_INTO_CABINET;
                gameTick(3);
                od_draw(od_cbo1, 46, 140);
                gameTick(2);
                od_draw(od_cbcl, 46, 140);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(2);
        }
}
