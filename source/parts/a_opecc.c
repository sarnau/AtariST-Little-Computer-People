/*
 * parts/a_opecc.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * delivery functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

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
        } else if (oc_stat != 0) {      /* STX re-tests the argument */
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
