/*
 * parts/a_gioob.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * ahouse functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_gioob()
{
        /* STX tests the call in place -- no local. */

        pst_arr[0] = STATE_UNDRESS_AT_BED;
        pst_arr[1] = STATE_LIE_DOWN_GETTING_IN;
        pst_arr[2] = STATE_LIE_DOWN_IN_BED;

        if (lcp.is_sleeping == NO) {
                hs_posXY(POS_MID_BED,
                                      &g_wtx, &g_wty);
                if (lcp_wkD() != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_IDLE;
                g_hatas = 10;
                lcp_hwt();
                lcp.is_sleeping = YES;
                lcp_x -= 10;
                lcp_face = FACING_RIGHT;
                lcp_st = pst_arr[0]; gameTick(2);
                lcp_x -= 8;
                lcp_st = pst_arr[1]; gameTick(2);
                lcp_x -= 2;
                lcp_st = pst_arr[2]; gameTick(2);
        } else {
                lcp_face = FACING_RIGHT;
                lcp_x += 10;
                lcp_st = pst_arr[1]; gameTick(2);
                lcp_x += 10;
                lcp_st = pst_arr[0]; gameTick(2);
                lcp.is_sleeping = NO;
                lcp_st              = STATE_STAND_IDLE;
                g_hatas = 10;
                lcp_hwt();
                gameTick(2);
        }
}
