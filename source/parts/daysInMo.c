/*
 * Included by stx_u2.c; never compiled on its own.
 */
/* The leap-year check reads the global t_year, not the `year`
   parameter.  Kept as in the original. */
short
daysInMo(month, year)
short   month;
short   year;
{
        /* No local: the test is inverted so the table lookup is the
           then-arm, and every arm returns directly. */
        if (month != 1)
                return days_pmo[month];
        else if ((t_year % 4) == 0)
                return 29;
        else
                return 28;
}
