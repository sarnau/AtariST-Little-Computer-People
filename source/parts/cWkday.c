/*
 * parts/cWkday.c -- shared body; LCP_STX links it in the 0xdece object
 * (0x1332e, in the 0xdece object). Files under parts/ are never
 * compiled standalone.
 */
/* The original references `daysInMo(dt_mon, dt_year)` inside the month
   loop instead of `daysInMo(i, dt_year)` -- preserved for fidelity
   though it's clearly a bug in the 1985 source.
   addr: cWkday() */
short
cWkday()
{
        /* STX carries only two locals (frame -8): it steps day_offset
           in place rather than routing it through `next_offset`, and
           accumulates the month lengths straight into it. */
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
