/*
 * parts/td_line.c -- shared body; LCP_STX links it in the 0xdece
 * object at 0x13c8a, immediately after tt_off. Files under parts/ are
 * never compiled standalone.
 */

void
td_line(color)
short   color;
{
        short   i;

        for (i = 0; i < 5; i++)
                drwLine(i + 44, 51 - (i >> 1),
                          i + 44, 57 - (i >> 1),
                          color);
}
