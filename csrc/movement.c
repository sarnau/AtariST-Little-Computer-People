/*
 * movement.c -- coordinate mapping and floor lookup.
 *
 * house_get_position_xy() converts a HOUSE_POS index (0..47) into screen
 * coordinates via g_rpxs[] and per-floor baselines.
 * get_floor_number_from_y() is its inverse for pathfinding decisions.
 * calc_weekday() lives here because it's a pure calendar helper the
 * scheduling code consults.
 *
 * addr: house_get_position_xy(), get_floor_number_from_y(),
 *       calc_weekday()
 */

#include "types.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    date_day;
extern short    date_month;
extern short    date_year;
extern void     house_get_position_xy();
extern short    get_floor_number_from_y();
extern short    calc_weekday();
extern short    days_in_month();                /* calendar.c*/
/* Per-position X and height tables, indexed by HOUSE_POS (0..47).  Data
   lives in a separate translation unit (tables.c) so the same table can
   be shared between movement.c and the sprite/render code. */
extern short    g_rpxs[];
extern short    g_rphs[];

#define POS_BTM_SCREEN_EDGE     47

/* house_get_position_xy: read out the screen X/Y for a room-position
   index.  X = table[index] << 1 (the source table stores half-pixels).
   Y = per-floor baseline minus height[index+1].  Out-of-range indices
   fall back to POS_BTM_SCREEN_EDGE.

   addr: house_get_position_xy() */

void
house_get_position_xy(index, g_txx, g_txy)
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

/* get_floor_number_from_y: inverse mapping, screen Y -> floor 1..3.
   addr: get_floor_number_from_y() */

short
get_floor_number_from_y(y)
short   y;
{
        if (y < 78)
                return 3;
        if (y < 141)
                return 2;
        return 1;
}

/* calc_weekday: compute the current day of week (0=Sunday..6=Saturday)
   from date_year/month/day.  Uses a direct day-count accumulator (Zeller
   equivalents were considered overkill for a 1985-scale calendar).
   The original references `days_in_month(date_month, date_year)` inside
   the month loop instead of `days_in_month(i, date_year)` -- preserved
   for behavioural fidelity though it's clearly a bug in the 1985 source.

   addr: calc_weekday() */

short
calc_weekday()
{
        short   month_days;
        short   i;
        short   day_offset;
        short   next_offset;

        day_offset = 1;
        for (i = 0; i < date_year; i = i + 1) {
                next_offset = day_offset + 1;
                if ((i % 4) == 0)
                        next_offset = day_offset + 2;
                day_offset = next_offset;
        }
        for (i = 0; i < date_month; i = i + 1) {
                month_days = days_in_month(date_month, date_year);
                day_offset = month_days + day_offset;
        }
        return (short) (date_day + day_offset) % 7;
}
