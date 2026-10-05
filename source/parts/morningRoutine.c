/*
 * parts/morningRoutine.c -- the morning routine after the alarm.
 * Included by stx_u2.c; never compiled on its own.
 */

void
morningRoutine()
{
        /* No local: the tick count is passed straight through. */

        g_actif = YES;
        alarm_p = YES;
        gameTick(rndRng(40, 100));
        if (lcp.is_sleeping == YES)
                getInOutOfBed();

        g_actif = YES; wakeFromAlarm();
        g_actif = YES; takeShower();
        g_actif = YES; brushTeeth();
        g_actif = YES; changeClothes(0);
        g_actif = YES; cookMeal();
        g_actif = NO;
}
