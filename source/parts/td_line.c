/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Draws the picture on the TV set in the living room: five short
   vertical lines in the given colour.  td_nois calls it every tick
   with a random colour while the TV is on, and tt_off blanks it in
   white. */
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
