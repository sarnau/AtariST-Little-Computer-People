/*
 * movement.c -- coordinate mapping and floor lookup.
 *
 * hs_posXY() converts a HOUSE_POS index (0..47) into screen
 * coordinates via g_rpxs[] and per-floor baselines.
 * getFlrY() is its inverse for pathfinding decisions.
 * cWkday() lives here because it's a pure calendar helper the
 * scheduling code consults.
 *
 * addr: hs_posXY(), getFlrY(),
 *       cWkday()
 */

#include "types.h"
#include "calendar.h"
#include "globals.h"
#include "movement.h"
#include "tables.h"
/* Per-position X and height tables, indexed by HOUSE_POS (0..47).  Data
   lives in a separate translation unit (tables.c) so the same table can
   be shared between movement.c and the sprite/render code. */

#include "enums.h"

/* hs_posXY: read out the screen X/Y for a room-position
   index.  X = table[index] << 1 (the source table stores half-pixels).
   Y = per-floor baseline minus height[index+1].  Out-of-range indices
   fall back to POS_BTM_SCREEN_EDGE.

   addr: hs_posXY() */

void
hs_posXY(index, g_txx, g_txy)
short   index;
short   *g_txx;
short   *g_txy;
{
        short   floor_y_pos;

        if (index > 47)
                index = POS_BTM_SCREEN_EDGE;

        *g_txx = g_rpxs[index] << 1;

        if (index < 16)
                floor_y_pos = 77;
        else if (index < 32)
                floor_y_pos = 140;
        else
                floor_y_pos = 202;

        *g_txy = floor_y_pos - g_rphs[index + 1];
}

/* getFlrY: inverse mapping, screen Y -> floor 1..3.
   addr: getFlrY() */

short
getFlrY(y)
short   y;
{
        if (y < 78)
                return 3;
        if (y < 141)
                return 2;
        return 1;
}

/* cWkday: compute the current day of week (0=Sunday..6=Saturday)
   from dt_year/month/day.  Uses a direct day-count accumulator (Zeller
   equivalents were considered overkill for a 1985-scale calendar).
   The original references `daysInMo(dt_mon, dt_year)` inside
   the month loop instead of `daysInMo(i, dt_year)` -- preserved
   for behavioural fidelity though it's clearly a bug in the 1985 source.

   addr: cWkday() */

short
cWkday()
{
        short   month_days;
        short   i;
        short   day_offset;
        short   next_offset;

        day_offset = 1;
        for (i = 0; i < dt_year; i = i + 1) {
                next_offset = day_offset + 1;
                if ((i % 4) == 0)
                        next_offset = day_offset + 2;
                day_offset = next_offset;
        }
        for (i = 0; i < dt_mon; i = i + 1) {
                month_days = daysInMo(dt_mon, dt_year);
                day_offset = month_days + day_offset;
        }
        return (short) (date_day + day_offset) % 7;
}
