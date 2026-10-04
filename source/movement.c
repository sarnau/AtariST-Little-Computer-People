/* movement.c -- coordinate mapping and floor lookup. */

#include "types.h"
#include "calendar.h"
#include "globals.h"
#include "protos.h"
#include "tables.h"
#include "enums.h"

/* Convert a house position (HOUSE_POS, 0..47) to screen coordinates.
   Out-of-range indexes are treated as POS_BTM_SCREEN_EDGE.  X is
   g_rpxs (stored at half resolution) doubled; Y is the floor's
   baseline -- 77 for positions 0..15 (top floor), 140 for 16..31,
   202 for 32..47 (ground floor) -- minus the position's g_rphs
   offset.  Results go to *g_txx and *g_txy. */
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

        *g_txy = floor_y_pos - g_rphs[index];
}

/* getFlrY -> parts/getFlrY.c. */

/* cWkday -> parts/cWkday.c. */
