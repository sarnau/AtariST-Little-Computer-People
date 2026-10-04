/*
 * Included by stx_u2.c; never compiled on its own.
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
