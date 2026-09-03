/*
 * parts/a_readn.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * ahouse functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

void
a_readn()
{
        /* STX tests the call in place -- no local. */
        /* STX keeps the LIMIT at -2 and the counter at -4; it has no
           rnd local at all (the Random test is inline). */
        short           t;
        short           i;

        pst_arr[0] = STATE_READ_PAPER_HOLD;
        pst_arr[1] = STATE_READ_PAPER_TURN_PAGE;
        tt_on();
        hs_posXY(POS_TOP_ARMCHAIR,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        g_hamod         = HEAD_ANIM_READING;
        lcp_face   = FACING_LEFT;
        lcp_st              = STATE_SIT_IN_ARMCHAIR;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER;
        lcp_hwt();
        /* STX sets the counter before the coordinate steps and
           emits an addi #0 on lcp_x -- a no-op the original wrote. */
        t = 200;
        lcp_x += 0;
        lcp_y += 8;
        i = 0;

        while (i < t) {
                if (g_trel[0] != ACTION_NONE)
                        break;
                lcp_face = FACING_LEFT;
                lcp_st            = pst_arr[0];
                if ((Random() & 0xf) == 5)
                        lcp_st = pst_arr[1];
                gameTick(1);
                i++;
        }

        lcp_y -= 8;
        lcp_face = FACING_LEFT;
        lcp_st = STATE_SIT_IN_ARMCHAIR;
        gameTick(2);
        tt_off();
}
