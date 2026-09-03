/*
 * parts/a_readn.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, away from ahouse.c's other
 * functions, so the default build includes it from stx_u2.c in
 * STX order; FAITHFUL includes it back in ahouse.c.
 * Files under parts/ are never compiled standalone.
 */

void
a_readn()
{
        /* STX tests the call in place -- no local. */
#ifdef FAITHFUL
        short           result;
#endif
#ifdef FAITHFUL
        unsigned short  rnd;
        short           t;
#else
        /* STX keeps the LIMIT at -2 and the counter at -4; it has no
           rnd local at all (the Random test is inline). */
        short           t;
        short           i;
#endif

        pst_arr[0] = STATE_READ_PAPER_HOLD;
        pst_arr[1] = STATE_READ_PAPER_TURN_PAGE;
        tt_on();
        hs_posXY(POS_TOP_ARMCHAIR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
                return;
#else
        if (lcp_wkD() != 0)
                return;
#endif

        g_hamod         = HEAD_ANIM_READING;
        lcp_face   = FACING_LEFT;
        lcp_st              = STATE_SIT_IN_ARMCHAIR;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE | HEAD_ANIM_SHOWER;
        lcp_hwt();
#ifdef FAITHFUL
        lcp_y = lcp_y + 8;
#else
        /* STX sets the counter before the coordinate steps and
           emits an addi #0 on lcp_x -- a no-op the original wrote. */
        t = 200;
        lcp_x += 0;
        lcp_y += 8;
        i = 0;
#endif

#ifdef FAITHFUL
        t = 0;
        while (t < 200 && g_trel[0] == ACTION_NONE) {
#else
        while (i < t) {
                if (g_trel[0] != ACTION_NONE)
                        break;
#endif
                lcp_face = FACING_LEFT;
                lcp_st            = pst_arr[0];
#ifdef FAITHFUL
                rnd = (unsigned short) Random();
                if ((rnd & 0xf) == 5)
#else
                if ((Random() & 0xf) == 5)
#endif
                        lcp_st = pst_arr[1];
                gameTick(1);
#ifdef FAITHFUL
                t = t + 1;
#else
                i++;
#endif
        }

#ifdef FAITHFUL
        lcp_y = lcp_y - 8;
#else
        lcp_y -= 8;
#endif
        lcp_face = FACING_LEFT;
        lcp_st = STATE_SIT_IN_ARMCHAIR;
        gameTick(2);
        tt_off();
}
