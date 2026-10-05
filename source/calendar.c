/* calendar.c -- calendar helpers and midnight reset. */

#include "types.h"
#include "calendar.h"
#include "globals.h"

/* daysInMonth -> parts/daysInMonth.c. */

void
resetDailyFlags()
{
        lunchDone      = NO;
        dinnerDone     = NO;
        wakeupDone  = NO;
        bedtimeDone         = NO;
}
