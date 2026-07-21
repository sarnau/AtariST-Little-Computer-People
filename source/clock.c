/* clock.c -- analog clock hand renderer.  Clock face at (278, 85). */

#include "types.h"
#include "enums.h"
#include "clock.h"
#include "gfx_prim.h"
#include "globals.h"

/* Y offset uses index + 3 (quarter-turn phase shift) so the same
   15-entry table serves both axes.
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
