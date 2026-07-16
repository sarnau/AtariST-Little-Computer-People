/*
 * calendar.c -- calendar helpers and midnight reset.
 *
 * daysInMo() consults a 12-entry lookup table (days_pmo[]) for
 * non-February months and applies a divisible-by-4 leap-year rule for
 * February.  daily_rs() is called by sim.c at 00:00 to
 * re-arm the once-per-day action triggers (meals, wake, bedtime).
 */

#include "types.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    dt_year;
extern BOOL16   lunT_trg;
extern BOOL16   dinT_trg;
extern BOOL16   wkT_trg;
extern BOOL16   bedT_trg;
extern void     daily_rs();     /* ai.c      */
extern short    daysInMo();                /* calendar.c*/
/* Days per calendar month.  Index 0 = January.  February is patched at
   runtime by daysInMo() with the leap-year branch. */
short days_pmo[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* daysInMo: return the number of days in month `m` of year `y`.
   Note: the original binary reads `dt_year` (the global) rather than
   the `year` parameter during the leap-year check.  Preserved verbatim
   so save-file compatibility survives, but flagged here as almost
   certainly an oversight in the 1985 source.

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
