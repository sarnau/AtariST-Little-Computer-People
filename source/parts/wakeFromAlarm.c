/*
 * parts/wakeFromAlarm.c -- walk to the bedroom and clear the alarm flag.
 * Included by stx_u2.c; never compiled on its own.
 */

void
wakeFromAlarm()
{
        /* The walk call is tested inline, without a local. */

        posToXY(POS_MID_BEDROOM_WALK,
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() == 0) {
                resFacing   = FACING_RIGHT;
                animState              = STATE_STAND_FACING_SCREEN;
                headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
                waitHeadTurn();
                alarmRinging = NO;
        }
}
