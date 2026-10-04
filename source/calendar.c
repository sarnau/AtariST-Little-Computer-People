/* calendar.c -- calendar helpers and midnight reset. */

#include "types.h"
#include "calendar.h"
#include "globals.h"

/* daysInMo -> parts/daysInMo.c. */

void
daily_rs()
{
        lunT_trg      = NO;
        dinT_trg     = NO;
        wkT_trg  = NO;
        bedT_trg         = NO;
}
