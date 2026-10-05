/*
 * Included by stx_u2.c; never compiled on its own.
 */


/* Draw both hands of the wall clock, centred at (278,85), in `color'.
   The minute hand is picked by minute/5 from g_cmmip, the hour hand by
   hour%12 from g_chhop; each table holds the x offset at [i] and the
   y offset (upward) at [i+3].  redrawHands calls it once with the old
   time to erase and once with the new time to draw. */
void
drawHands(minute, hour, color)
short   minute;
short   hour;
short   color;
{
        short   dx;
        short   dy;
        short   m;
        short   h;

        m  = minute / 5;
        dx = g_cmmip[m];
        dy = g_cmmip[m + 3];
        drawLine(278, 85, 278 + dx, 85 - dy, color);

        h  = hour % 12;
        dx = g_chhop[h];
        dy = g_chhop[h + 3];
        drawLine(278, 85, 278 + dx, 85 - dy, color);
}
