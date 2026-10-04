/*
 * parts/a_opcfd.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* a_opcfd: open (DOOR_OPEN, 0) or close (DOOR_CLOSE) the front door
   while the resident stands at it.  Opening reaches out, draws the
   door ajar then wide with the door-open sound and sets lcp_frdO;
   closing draws it ajar then shut with the door-close sound and
   clears lcp_frdO.  A request matching the current state returns
   immediately. */
void
a_opcfd(door_st)
short   door_st;
{
        if (door_st == 0) {
                if (lcp_frdO != NO)
                        return;
                lcp_face = FACING_RIGHT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                od_draw(od_fro1, 294, 151);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(od_fro2, 294, 151);
                gameTick(2);
                lcp_frdO = YES;
        } else if (door_st != 0) {      /* redundant re-test, kept on purpose */
                if (lcp_frdO == NO)
                        return;
                od_draw(od_fro1, 294, 151);
                gameTick(2);
                od_draw(od_frcl, 294, 151);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                gameTick(2);
                lcp_frdO = NO;
        }
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
