/* calendar.c -- calendar helpers and midnight reset. */

#include "types.h"
#include "calendar.h"
#include "globals.h"

/* daysInMonth -> parts/daysInMonth.c. */

void
resetDailyFlags()
{
        lunT_trg      = NO;
        dinT_trg     = NO;
        wkT_trg  = NO;
        bedT_trg         = NO;
}
