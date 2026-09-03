/* calendar.c -- calendar helpers and midnight reset. */

#include "types.h"
#include "calendar.h"
#include "globals.h"

/* daysInMo -> parts/daysInMo.c (STX: 0x13796, in the 0xdece object just ahead of cWkday). */

/* addr: daily_rs() */
void
daily_rs()
{
        lunT_trg      = NO;
        dinT_trg     = NO;
        wkT_trg  = NO;
        bedT_trg         = NO;
}
