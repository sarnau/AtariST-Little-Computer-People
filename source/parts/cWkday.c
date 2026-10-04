/*
 * parts/cWkday.c -- included by stx_u2.c; never compiled on its own.
 */
/* The original references `daysInMo(dt_mon, dt_year)` inside the month
   loop instead of `daysInMo(i, dt_year)` -- a bug in the 1985 source,
   kept on purpose. */
short
cWkday()
{
        /* Only two locals: day_offset is stepped in place and the
           month lengths accumulate straight into it. */
        short   day_offset;
        short   i;

        day_offset = 1;
        for (i = 0; i < dt_year; i++) {
                day_offset++;
                if ((i % 4) == 0)
                        day_offset++;
        }
        for (i = 0; i < dt_mon; i++)
                day_offset += daysInMo(dt_mon, dt_year);
        day_offset += date_day;
        day_offset %= 7;
        return day_offset;
}
