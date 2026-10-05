/*
 * render.c -- VDI palette and screen refresh.
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

/* setSkinColor -> renderx.c */

/* renderFrame -> renderf.c */

/* redrawHands: erase prev hands in white, draw new pair in grey.
   Skips when t_min hasn't advanced past cached clockMinute. */


/* drawObject: blit background object at (x,y) through the game's own
   vro_cpy binding. */


void
drawObject(g_oiidx, x, y)
short   g_oiidx;
short   x;
short   y;
{
        blitRect(vdiHandle, 3,
                /* Addresses the MFDB array itself (20 bytes per entry),
                   not through a pointer variable. */
                g_oiidx * 20 + (long) objMfdbs,
                (long) &houseMfdb,
                0, 0,
                objWidths[g_oiidx] - 1,
                objHeights[g_oiidx] - 1,
                x, y,
                objWidths[g_oiidx] + x - 1,
                objHeights[g_oiidx] + y - 1);
}

/* fillPanel: clear top text strip (rows 0..maxY-1).
   White fill for letter pane (maxY < 70), striped house-bg fill otherwise.
   Last row painted black as separator.
   Lives in parts/fillPanel.c (stx_u1's object). */

/* scrollStrip, tvNoise, animRecPlayer -> renderx.c */

/* -- TV toggle -- */

/* tvOn: walk to living room, idle look-left, set flag, play click SFX.
   Returns -1 on walk failure, 0 otherwise.  Lives in parts/tvOn.c. */

/* tvOff: same walk, clear flag, redraw antenna in off state.
   Note: the 1985 code plays no SFX_TV_CLICK when switching off --
   kept on purpose.  Lives in parts/tvOff.c. */

/* -- Kitchen food-cabinet overlay -- */

/* drawFoodCab: paint food-count markers in 4 cabinet slots.
   Count = bits 9..11 of door_states_and_flags (0..4 packs). No-op if closed.
     1 -> (50,159)  2 -> (58,159)  3 -> (50,151)  4 -> (58,151) */

void
drawFoodCab()
{
        short           cabinet_content;    /* signed on purpose: the compares must be signed */

        if (kitchenCabOpen == NO)
                return;

        cabinet_content = (resident.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD;
        drawObject(OBJ_CABINET_OPEN_2, KITCHEN_CAB_X, KITCHEN_CAB_Y);

        if (cabinet_content >= 1) drawObject(OBJ_CABINET_ITEM, 50, 159);
        if (cabinet_content >= 2) drawObject(OBJ_CABINET_ITEM, 58, 159);
        if (cabinet_content >= 3) drawObject(OBJ_CABINET_ITEM, 50, 151);
        if (cabinet_content >= 4) drawObject(OBJ_CABINET_ITEM, 58, 151);
}

/* -- Water tank level bar (VDI polylines) -- */

/* updateWaterTank: repaint/animate water tank indicator at x=146..159, y=165..174.
     val == 0 : full redraw at current waterLevel
     val <  0 : drain `-val` steps, one game-tick each
     val >  0 : fill `val` steps
   Each level = one 14px horizontal polyline; colour 0x0D filled, 0x0C empty.
   VDI ops go to backbuffer so animation isn't torn by next 8Hz render.
   Lives in parts/updateWaterTank.c. */
