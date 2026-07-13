/*
 * clock.c -- analog clock hand renderer.
 *
 * The clock face lives at (278, 85) on the top status strip.  Each
 * hand is a single straight line from the centre to a point on a
 * radius-14 circle.  The 6-entry g_cmmip and 24-entry
 * g_chhop tables split X (first half) from Y (second
 * half) so the same table indexes both a horizontal and vertical
 * offset for a given time step.
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
   minute/5 picks one of 6 X/Y pairs (12 clock ticks total, doubled up:
   :00/:05/:10/:15/... share offset entries).  hour%12 picks one of 12
   pairs directly.  X offsets: entries 0..5 / 0..11; Y offsets: entries
   3..5 / 12..23 (same table, +3/+12 stride).
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
