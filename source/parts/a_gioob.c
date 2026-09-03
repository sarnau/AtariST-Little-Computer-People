/*
 * parts/a_gioob.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from ahouse.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in ahouse.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_gioob()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short   result;
#endif

        pst_arr[0] = STATE_UNDRESS_AT_BED;
        pst_arr[1] = STATE_LIE_DOWN_GETTING_IN;
        pst_arr[2] = STATE_LIE_DOWN_IN_BED;

        if (lcp.is_sleeping == NO) {
                hs_posXY(POS_MID_BED,
                                      &g_wtx, &g_wty);
#ifdef FAITHFUL
                result = lcp_wkD();
                if (result != 0)
                        return;
#else
                if (lcp_wkD() != 0)
                        return;
#endif
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_IDLE;
                g_hatas = 10;
                lcp_hwt();
                lcp.is_sleeping = YES;
#ifdef FAITHFUL
                lcp_x = lcp_x - 10;
#else
                lcp_x -= 10;
#endif
                lcp_face = FACING_RIGHT;
                lcp_st = pst_arr[0]; gameTick(2);
#ifdef FAITHFUL
                lcp_x = lcp_x - 8;
#else
                lcp_x -= 8;
#endif
                lcp_st = pst_arr[1]; gameTick(2);
#ifdef FAITHFUL
                lcp_x = lcp_x - 2;
#else
                lcp_x -= 2;
#endif
                lcp_st = pst_arr[2]; gameTick(2);
        } else {
                lcp_face = FACING_RIGHT;
#ifdef FAITHFUL
                lcp_x = lcp_x + 10;
#else
                lcp_x += 10;
#endif
#ifdef FAITHFUL
                lcp_st = STATE_LIE_DOWN_GETTING_IN; gameTick(2);
#else
                lcp_st = pst_arr[1]; gameTick(2);
#endif
#ifdef FAITHFUL
                lcp_x = lcp_x + 10;
#else
                lcp_x += 10;
#endif
                lcp_st = pst_arr[0]; gameTick(2);
                lcp.is_sleeping = NO;
                lcp_st              = STATE_STAND_IDLE;
                g_hatas = 10;
                lcp_hwt();
                gameTick(2);
        }
}
