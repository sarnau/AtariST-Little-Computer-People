/*
 * parts/a_opecd.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from adoors.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in adoors.c.
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
#ifdef FAITHFUL
        } else {
#else
        } else if (oc_stat != 0) {      /* STX re-tests the argument */
#endif
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
