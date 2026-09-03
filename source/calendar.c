/* calendar.c -- calendar helpers and midnight reset. */

#include "types.h"
#include "calendar.h"
#include "globals.h"

short days_pmo[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

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
