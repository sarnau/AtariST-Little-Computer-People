/*
 * parts/a_opcfd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from delivery.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in delivery.c.
 * Files under parts/ are never compiled standalone.
 */

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
#ifdef FAITHFUL
        } else {
#else
        } else if (door_st != 0) {      /* STX re-tests the argument */
#endif
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
