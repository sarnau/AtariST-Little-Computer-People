/* Clear the four once-a-day triggers so lunch, dinner, wake-up and
   bedtime can fire again; called when the clock passes midnight. */
void
resetDailyFlags()
{
        lunchDone = NO;
        dinnerDone = NO;
        wakeupDone = NO;
        bedtimeDone = NO;
}
