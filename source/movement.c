/* movement.c -- coordinate mapping and floor lookup. */

#include "types.h"
#include "calendar.h"
#include "globals.h"
#include "movement.h"
#include "tables.h"
#include "enums.h"

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
