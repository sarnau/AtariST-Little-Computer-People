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

        /* The other revision reads one slot past the position index; the STX
           revision indexes the height table directly. */
        *g_txy = floor_y_pos - g_rphs[index];
}

/* getFlrY -> parts/getFlrY.c (STX: 0x5224, after lcp_flwp). */

/* cWkday -> parts/cWkday.c (STX: 0x1332e, in the 0xdece object). */
