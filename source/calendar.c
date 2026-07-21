/* calendar.c -- calendar helpers and midnight reset. */

#include "types.h"
#include "calendar.h"
#include "globals.h"

short days_pmo[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* The original binary reads dt_year (global) rather than the `year`
   parameter during the leap-year check.  Preserved verbatim for
   save-file compatibility.
   addr: daysInMo() */
short
daysInMo(month, year)
short   month;
short   year;
{
        short   result;

        if (month == 1) {
                if ((dt_year % 4) == 0)
                        result = 29;
                else
                        result = 28;
        } else {
                result = days_pmo[month];
        }
        return result;
}

/* daily_rs: called from gameSim1() when
   the clock rolls over to 00:00.  Clears the four "already fired today"
   flags so meals, wake-up, and bedtime can trigger again on the new
   game-day.

   addr: daily_rs() */

void
daily_rs()
{
        lunT_trg      = NO;
        dinT_trg     = NO;
        wkT_trg  = NO;
        bedT_trg         = NO;
}
