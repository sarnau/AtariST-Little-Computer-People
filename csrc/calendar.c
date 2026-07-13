/*
 * calendar.c -- calendar helpers and midnight reset.
 *
 * days_in_month() consults a 12-entry lookup table (days_per_month[]) for
 * non-February months and applies a divisible-by-4 leap-year rule for
 * February.  daily_reset_action_flags() is called by sim.c at 00:00 to
 * re-arm the once-per-day action triggers (meals, wake, bedtime).
 */

#include "types.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    date_year;
extern BOOL16   lunch_meal_triggered_today;
extern BOOL16   dinner_meal_triggered_today;
extern BOOL16   morning_wakeup_triggered_today;
extern BOOL16   bedtime_triggered_today;
extern void     daily_reset_action_flags();     /* ai.c      */
extern short    days_in_month();                /* calendar.c*/
/* Days per calendar month.  Index 0 = January.  February is patched at
   runtime by days_in_month() with the leap-year branch. */
short days_per_month[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* days_in_month: return the number of days in month `m` of year `y`.
   Note: the original binary reads `date_year` (the global) rather than
   the `year` parameter during the leap-year check.  Preserved verbatim
   so save-file compatibility survives, but flagged here as almost
   certainly an oversight in the 1985 source.

   addr: days_in_month() */

short
days_in_month(month, year)
short   month;
short   year;
{
        short   result;

        if (month == 1) {
                if ((date_year % 4) == 0)
                        result = 29;
                else
                        result = 28;
        } else {
                result = days_per_month[month];
        }
        return result;
}

/* daily_reset_action_flags: called from game_simulate_one_second() when
   the clock rolls over to 00:00.  Clears the four "already fired today"
   flags so meals, wake-up, and bedtime can trigger again on the new
   game-day.

   addr: daily_reset_action_flags() */

void
daily_reset_action_flags()
{
        lunch_meal_triggered_today      = NO;
        dinner_meal_triggered_today     = NO;
        morning_wakeup_triggered_today  = NO;
        bedtime_triggered_today         = NO;
}
