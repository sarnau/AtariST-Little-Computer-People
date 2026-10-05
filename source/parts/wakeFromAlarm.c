/*
 * parts/wakeFromAlarm.c -- walk to the bedroom and clear the alarm flag.
 * Included by stx_u2.c; never compiled on its own.
 */

void
wakeFromAlarm()
{
        /* The walk call is tested inline, without a local. */

        posToXY(POS_MID_BEDROOM_WALK,
                              &g_wtx, &g_wty);
        if (walkToTarget() == 0) {
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                waitHeadTurn();
                alarm_p = NO;
        }
}
