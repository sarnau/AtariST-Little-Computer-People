/*
 * parts/a_sleep.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * aidle functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */


void
a_sleep(value)
short   value;
{
        short   i;
        short   duration;

        pst_arr[0] = STATE_SLP_BREATHE_I;
        pst_arr[1] = STATE_SLP_BREATHE_O;

        if (lcp_stR != NO)
                return;

        if (value == -1) {
                g_wtx = lcp_x;
                g_wty = flr_cy[getFlrY(lcp_y) - 1];
                if (lcp_wkD() != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_SIDE_VIEW;
                g_hatas = 8;
                lcp_hwt();
        }

        duration = rndRng(7, 15);
        if (value != -1)
                duration = value;

        i = 0;
        while (i < duration) {
                if (g_trel[0] != ACTION_NONE)
                        break;
                lcp_st = pst_arr[0]; gameTick(1);
                lcp_st = pst_arr[1]; gameTick(0);
                sf_sele(SFX_SNORING, 3L);
                gameTick(1);
                lcp_st = pst_arr[0]; gameTick(1);
                i++;
        }

        if (value == -1) {
                lcp_st = STATE_STAND_SIDE_VIEW;
                gameTick(0);
        }
}
