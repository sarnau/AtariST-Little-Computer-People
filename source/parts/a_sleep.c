/*
 * Included by stx_u2.c; never compiled on its own.
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

        if (value == SLEEP_RANDOM) {
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
        if (value != SLEEP_RANDOM)
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

        if (value == SLEEP_RANDOM) {
                lcp_st = STATE_STAND_SIDE_VIEW;
                gameTick(0);
        }
}
