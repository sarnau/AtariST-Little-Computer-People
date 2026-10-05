/* movement.c -- coordinate mapping and floor lookup. */

#include "types.h"
#include "calendar.h"
#include "globals.h"
#include "protos.h"
#include "tables.h"
#include "enums.h"

/* Convert a house position (POS_*, 0..47: sixteen per floor, top floor
   first) to screen coordinates.
   Out-of-range indexes are treated as POS_BTM_SCREEN_EDGE.  X is
   posXHalf (stored at half resolution) doubled; Y is the floor's
   baseline -- 77 for positions 0..15 (top floor), 140 for 16..31,
   202 for 32..47 (ground floor) -- minus the position's posYOffset
   offset.  Results go to *xOut and *yOut. */
void
posToXY(index, xOut, yOut)
short   index;
short   *xOut;
short   *yOut;
{
        short   floorYPos;

        if (index > POS_BTM_SCREEN_EDGE)
                index = POS_BTM_SCREEN_EDGE;

        *xOut = posXHalf[index] << 1;

        if (index < POS_PER_FLOOR)
                floorYPos = 77;
        else if (index < 2 * POS_PER_FLOOR)
                floorYPos = 140;
        else
                floorYPos = 202;

        *yOut = floorYPos - posYOffset[index];
}

