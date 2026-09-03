/*
 * parts/a_opcfd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * delivery functions, so stx_u2.c includes it in LCP_STX order.
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
        } else if (door_st != 0) {      /* STX re-tests the argument */
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
