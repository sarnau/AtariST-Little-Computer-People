/*
 * clock.c -- analog clock hand renderer.
 *
 * The clock face lives at (278, 85) on the top status strip.  Each
 * hand is a single straight line from the centre to a point on a
 * small circle around it.  g_cmmip / g_chhop are 15-entry tables:
 * each hand indexes X at position [t] and Y at [t + 3], which is the
 * same circle offset by a quarter turn (90 degree phase shift).
 *
 * addr: clock_draw_hands()
 */

#include "types.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    g_cmmip[];
extern short    g_chhop[];
extern void     draw_line();

/* clock_draw_hands: paint the minute + hour hands in `color`.
   minute/5 in 0..11 picks a position on the minute circle; hour%12
   picks a position on the hour circle.  Y offset uses index + 3
   (quarter-turn phase shift) so the same table serves both axes.
   addr: clock_draw_hands() */

void
clock_draw_hands(minute, hour, color)
short   minute;
short   hour;
short   color;
{
        short   m;
        short   h;

        m = minute / 5;
        h = hour % 12;

        draw_line(278, 85,
                  278 + g_cmmip[m],
                   85 - g_cmmip[m + 3],
                  color);
        draw_line(278, 85,
                  278 + g_chhop[h],
                   85 - g_chhop[h + 3],
                  color);
}
