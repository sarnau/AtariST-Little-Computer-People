/*
 * parts/daysInMo.c -- shared body; LCP_ORG links it in calendar.c,
 * LCP_STX in the 0xdece object (0x13796, in the 0xdece object just ahead of cWkday).  Files under parts/
 * are never compiled standalone.
 */
/* The original binary reads dt_year (global) rather than the `year`
   parameter during the leap-year check.  Preserved verbatim for
   save-file compatibility.
   addr: daysInMo() */
short
daysInMo(month, year)
short   month;
short   year;
{
#ifdef FAITHFUL
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
#else
        /* STX has no local: the test is inverted so the table lookup
           is the then-arm, and every arm returns directly. */
        if (month != 1)
                return days_pmo[month];
        else if ((dt_year % 4) == 0)
                return 29;
        else
                return 28;
#endif
}
