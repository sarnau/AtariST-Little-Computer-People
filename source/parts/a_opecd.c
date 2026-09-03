/*
 * parts/a_opecd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * adoors functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

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
                od_draw(od_dro1, 97, 115);
                gameTick(2);
                od_draw(od_dro2, 97, 115);
                gameTick(2);
        } else if (oc_stat != 0) {      /* STX re-tests the argument */
                if (lcp_drsO == NO)
                        return;
                lcp_drsO = NO;
                lcp_st = STATE_BEND_DOWN;    gameTick(1);
                lcp_st = STATE_REACH_FORWARD;gameTick(2);
                od_draw(od_dro1, 97, 115);
                gameTick(2);
                od_draw(od_drcl, 97, 115);
                gameTick(2);
        }
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
}
