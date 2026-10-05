/*
 * parts/morningRoutine.c -- the morning routine after the alarm.
 * Included by stx_u2.c; never compiled on its own.
 */

void
morningRoutine()
{
        /* No local: the tick count is passed straight through. */

        noPreempt = YES;
        alarmRinging = YES;
        gameTick(rndRng(40, 100));
        if (resident.is_sleeping == YES)
                getInOutOfBed();

        noPreempt = YES; wakeFromAlarm();
        noPreempt = YES; takeShower();
        noPreempt = YES; brushTeeth();
        noPreempt = YES; changeClothes(0);
        noPreempt = YES; cookMeal();
        noPreempt = NO;
}
