/* The original references `daysInMonth(t_mon, t_year)` inside the month
   loop instead of `daysInMonth(i, t_year)` -- a bug in the 1985 source,
   kept on purpose. */
short
calcWeekday()
{
        /* Only two locals: day_offset is stepped in place and the
           month lengths accumulate straight into it. */
        short   day_offset;
        short   i;

        day_offset = 1;
        for (i = 0; i < t_year; i++) {
                day_offset++;
                if ((i % 4) == 0)
                        day_offset++;
        }
        for (i = 0; i < t_mon; i++)
                day_offset += daysInMonth(t_mon, t_year);
        day_offset += t_day;
        day_offset %= 7;
        return day_offset;
}
