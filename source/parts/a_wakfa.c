/*
 * parts/a_wakfa.c -- walk to the bedroom and clear the alarm flag.
 * Included by stx_u2.c; never compiled on its own.
 */

void
a_wakfa()
{
        /* The walk call is tested inline, without a local. */

        hs_posXY(POS_MID_BEDROOM_WALK,
                              &g_wtx, &g_wty);
        if (lcp_wkD() == 0) {
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                alarm_p = NO;
        }
}
