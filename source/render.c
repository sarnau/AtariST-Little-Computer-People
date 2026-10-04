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

/* lcp_upal -> renderx.c */

/* sc_ren8 -> renderf.c */

/* cl_redrH: erase prev hands in white, draw new pair in grey.
   Skips when t_min hasn't advanced past cached g_cmmin. */


/* od_draw: blit background object at (x,y) through the game's own
   vro_cpy binding. */


void
od_draw(g_oiidx, x, y)
short   g_oiidx;
short   x;
short   y;
{
        vroCpyD(vdihnd, 3,
                /* Addresses the MFDB array itself (20 bytes per entry),
                   not through a pointer variable. */
                g_oiidx * 20 + (long) g_obtmt,
                (long) &mf_scrp,
                0, 0,
                g_obtaw[g_oiidx] - 1,
                g_obtah[g_oiidx] - 1,
                x, y,
                g_obtaw[g_oiidx] + x - 1,
                g_obtah[g_oiidx] + y - 1);
}

/* fillTopR: clear top text strip (rows 0..maxY-1).
   White fill for letter pane (maxY < 70), striped house-bg fill otherwise.
   Last row painted black as separator.
   Lives in parts/fillTopR.c (stx_u1's object). */

/* sc_sctd, td_nois, rp_anim -> renderx.c */

/* -- TV toggle -- */

/* tt_on: walk to living room, idle look-left, set flag, play click SFX.
   Returns -1 on walk failure, 0 otherwise.  Lives in parts/tt_on.c. */

/* tt_off: same walk, clear flag, redraw antenna in off state.
   Note: the 1985 code plays no SFX_TV_CLICK when switching off --
   kept on purpose.  Lives in parts/tt_off.c. */

/* -- Kitchen food-cabinet overlay -- */

/* sc_drfc: paint food-count markers in 4 cabinet slots.
   Count = bits 9..11 of door_states_and_flags (0..4 packs). No-op if closed.
     1 -> (50,159)  2 -> (58,159)  3 -> (50,151)  4 -> (58,151) */

void
sc_drfc()
{
        short           cabinet_content;    /* signed on purpose: the compares must be signed */

        if (lcp_cabO == NO)
                return;

        cabinet_content = (lcp.door_states_and_flags >> DSF_FOOD_SHIFT) & DSF_FOOD_FIELD;
        od_draw(OBJ_CABINET_OPEN_2, KITCHEN_CAB_X, KITCHEN_CAB_Y);

        if (cabinet_content >= 1) od_draw(OBJ_CABINET_ITEM, 50, 159);
        if (cabinet_content >= 2) od_draw(OBJ_CABINET_ITEM, 58, 159);
        if (cabinet_content >= 3) od_draw(OBJ_CABINET_ITEM, 50, 151);
        if (cabinet_content >= 4) od_draw(OBJ_CABINET_ITEM, 58, 151);
}

/* -- Water tank level bar (VDI polylines) -- */

/* updWtLv: repaint/animate water tank indicator at x=146..159, y=165..174.
     val == 0 : full redraw at current lcp_watr
     val <  0 : drain `-val` steps, one game-tick each
     val >  0 : fill `val` steps
   Each level = one 14px horizontal polyline; colour 0x0D filled, 0x0C empty.
   VDI ops go to backbuffer so animation isn't torn by next 8Hz render.
   Lives in parts/updWtLv.c. */
