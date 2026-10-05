/*
 * render.c -- drawObject, and the kitchen cabinet's food overlay.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#ifdef HOST
#include "hostgem.h"
#else
#include <vdibind.h>
#endif
#include "vdiown.h"
#include "obdefs1.h"
#include "protos.h"
#include "globals.h"

/* drawObject: copy object frame obj (an OBJ_* id) from the OBJECTS
   bitmaps onto the house picture at (x, y), replace mode, through the
   game's own vro_cpyfm binding. */

void
drawObject(obj, x, y)
short   obj;
short   x;
short   y;
{
        blitRect(vdiHandle, S_ONLY,
                (long) &objMfdbs[obj],
                (long) &houseMfdb,
                0, 0,
                objWidths[obj] - 1,
                objHeights[obj] - 1,
                x, y,
                objWidths[obj] + x - 1,
                objHeights[obj] + y - 1);
}

/* -- Kitchen food-cabinet overlay -- */

/* drawFoodCab: paint food-count markers in 4 cabinet slots.
   Count = bits 9..11 of doorStatesAndFlags (0..4 packs). No-op if closed.
     1 -> (50,159)  2 -> (58,159)  3 -> (50,151)  4 -> (58,151) */

void
drawFoodCab()
{
        short           cabinetContent;    /* signed on purpose: the compares must be signed */

        if (kitchenCabOpen == NO)
                return;

        cabinetContent = (resident.doorStatesAndFlags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD;
        drawObject(OBJ_CABINET_OPEN_2, KITCHEN_CAB_X, KITCHEN_CAB_Y);

        if (cabinetContent >= 1) drawObject(OBJ_CABINET_ITEM, 50, 159);
        if (cabinetContent >= 2) drawObject(OBJ_CABINET_ITEM, 58, 159);
        if (cabinetContent >= 3) drawObject(OBJ_CABINET_ITEM, 50, 151);
        if (cabinetContent >= 4) drawObject(OBJ_CABINET_ITEM, 58, 151);
}
