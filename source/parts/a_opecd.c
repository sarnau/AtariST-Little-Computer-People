/*
 * Included by stx_u2.c; never compiled on its own.
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
        } else if (oc_stat != 0) {      /* redundant re-test, kept on purpose */
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
