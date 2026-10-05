/* The original references `daysInMonth(t_mon, t_year)` inside the month
   loop instead of `daysInMonth(i, t_year)` -- a bug in the 1985 source,
   kept on purpose. */
short
calcWeekday()
{
        /* Only two locals: dayOffset is stepped in place and the
           month lengths accumulate straight into it. */
        short   dayOffset;
        short   i;

        dayOffset = 1;
        for (i = 0; i < t_year; i++) {
                dayOffset++;
                if ((i % 4) == 0)
                        dayOffset++;
        }
        for (i = 0; i < t_mon; i++)
                dayOffset += daysInMonth(t_mon, t_year);
        dayOffset += t_day;
        dayOffset %= 7;
        return dayOffset;
}
