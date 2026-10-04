/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* a_readn: read the newspaper.  The TV is switched on first (tt_on),
   then the resident walks to the armchair, sits, and reads for up to
   200 ticks -- holding the paper and turning a page about one tick in
   sixteen -- until a new action is queued.  He is lowered 8 pixels
   into the chair for the reading poses and raised again afterwards,
   and the TV is switched off at the end. */
void
a_readn()
{
        /* The limit is declared before the counter, and the walk call
           and the Random test are inline with no local; all as in the
           original. */
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
        /* The limit is set before the coordinate steps, and
           `lcp_x += 0` is a no-op the original wrote.  Both kept on
           purpose. */
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
