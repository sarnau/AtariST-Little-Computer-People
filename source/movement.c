/*
 * movement.c -- coordinate mapping and floor lookup.
 * addr: hs_posXY(), getFlrY(), cWkday()
 */

#include "types.h"
#include "calendar.h"
#include "globals.h"
#include "movement.h"
#include "tables.h"
#include "enums.h"

/* addr: hs_posXY() */
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

        /* LCP_ORG reads one slot past the position index; the STX
           revision indexes the height table directly. */
#ifdef FAITHFUL
        *g_txy = floor_y_pos - g_rphs[index + 1];
#else
        *g_txy = floor_y_pos - g_rphs[index];
#endif
}

/* addr: getFlrY() */
short
getFlrY(y)
short   y;
{
#ifdef FAITHFUL
        if (y < 78)
                return 3;
        if (y < 141)
                return 2;
        return 1;
#else
        /* STX: one if/else-if/else chain -- each arm's return is
           followed by the else-skip branch. */
        if (y > 140)
                return 1;
        else if (y > 77)
                return 2;
        else
                return 3;
#endif
}

/* The original references `daysInMo(dt_mon, dt_year)` inside the month
   loop instead of `daysInMo(i, dt_year)` -- preserved for fidelity
   though it's clearly a bug in the 1985 source.
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
