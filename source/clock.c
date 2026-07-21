/*
 * clock.c -- analog clock hand renderer.
 *
 * The clock face lives at (278, 85) on the top status strip.  Each
 * hand is a single straight line from the centre to a point on a
 * small circle around it.  g_cmmip / g_chhop are 15-entry tables:
 * each hand indexes X at position [t] and Y at [t + 3], which is the
 * same circle offset by a quarter turn (90 degree phase shift).
 *
 * addr: cl_drwH()
 */

#include "types.h"
#include "enums.h"
#include "clock.h"
#include "gfx_prim.h"
#include "globals.h"

/* cl_drwH: paint the minute + hour hands in `color`.
   minute/5 in 0..11 picks a position on the minute circle; hour%12
   picks a position on the hour circle.  Y offset uses index + 3
   (quarter-turn phase shift) so the same table serves both axes.
   addr: cl_drwH() */

void
cl_drwH(minute, hour, color)
short   minute;
short   hour;
short   color;
{
        short   m;
        short   h;

        m = minute / 5;
        h = hour % 12;

        drwLine(278, 85,
                  278 + g_cmmip[m],
                   85 - g_cmmip[m + 3],
                  color);
        drwLine(278, 85,
                  278 + g_chhop[h],
                   85 - g_chhop[h + 3],
                  color);
}
