/*
 * render.c -- VDI palette and screen refresh (stubs for now).
 *
 * sc_ren8() copies the pending sprite buffers onto the visible
 * screen at ~8 Hz.  lcp_upal() reloads the 16-entry
 * palette from lcp_current_palette[] via Setpalette(); called after
 * sickness onset, sickness recovery, and TV toggle.
 *
 * All hardware calls (Setpalette, VDI vs_color) will be reintroduced
 * when the render pipeline is ported for real; today these are stubs so
 * everything upstream links.
 *
 * addr: sc_ren8(), lcp_upal()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <vdibind.h>
#include <obdefs.h>
#include "ahouse.h"
#include "clock.h"
#include "gfx_prim.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "sound.h"
#include "tick.h"
#include "walk.h"

/* lcp_upal -> renderx.c */

/* sc_ren8 -> renderf.c */

/* cl_redrH: erase the previous minute/hour hand pair (paint
   in white), then draw the new pair (paint in grey).  Compares
   t_min against a cached g_cmmin to skip work when the
   clock hasn't advanced yet.
   addr: cl_redrH() */


void
cl_redrH()
{
        if (g_cmmin == t_min)
                return;
        cl_drwH(g_cmmin, g_chhou, COLOR_white);
        g_cmmin = t_min;
        g_chhou   = t_hour;
        cl_drwH(t_min, t_hour, COLOR_grey);
}

/* od_draw: blit a pre-loaded background object at (x, y) via
   vro_cpyfm (VRO copy S_ONLY = replace, no transparency).  Each
   object has its width/height stored in g_obtaw/height[] and
   its MFDB source rect in g_otmfd offset by g_oiidx.
   addr: od_draw() */


void
od_draw(g_oiidx, x, y)
short   g_oiidx;
short   x;
short   y;
{
        /* Ghidra: `object_tab_mfdb + object_index` (MFDB* pointer
           arithmetic).  Our port had `(char*)g_otmfd + g_oiidx`
           which treats g_oiidx as a byte offset instead of an array
           index -- MFDB is 18 bytes so all non-zero object IDs
           landed misaligned and VDI got junk width/height. */
        {
                short   pxy[8];
                pxy[0] = 0;
                pxy[1] = 0;
                pxy[2] = g_obtaw[g_oiidx] - 1;
                pxy[3] = g_obtah[g_oiidx] - 1;
                pxy[4] = x;
                pxy[5] = y;
                pxy[6] = x + g_obtaw[g_oiidx] - 1;
                pxy[7] = y + g_obtah[g_oiidx] - 1;
                vro_cpyfm(vdihnd, S_ONLY, pxy,
                          &((MFDB *) g_otmfd)[g_oiidx],
                          &mf_scrp);
        }
}

/* fillTopR: clear the top text strip (rows 0
   through maxY-1).  Uses a solid white fill for the letter-typing
   pane (maxY < 70) and a striped house-background fill for the
   larger clear cases.  The last row before maxY is painted black to
   form a separator.
   addr: fillTopR() */


void
fillTopR(max_y)
short   max_y;
{
        short   y;

        /* dest_screenbase_ptr = (dest_scr_buffer + 0x200) & ~0x1FF,
           precomputed in stpScrB.  Ghidra shows the
           fold as `dest_scr_buffer + 0x7f` (the +0xFE-byte residual
           after the align-up-to-512 lands on our BSS base). */
        g_dscp = (void *) g_dsb;

        for (y = 0; y < max_y - 1; y = y + 1) {
                if (max_y < 70)
                        sc_firw(g_dscp, y);
                else
                        sc_firs(g_dscp, y);
        }
        sc_firb(g_dscp, max_y - 1);
}

/* sc_sctd, td_nois -> renderx.c */

/* rp_anim -> renderx.c */

/* ---- TV toggle ------------------------------------------------------- */

/* tt_on: walk to the living room, do an idle look-left, set the
   flag and play the click SFX.  Returns -1 on walk failure, 0 otherwise.
   addr: tt_on() */

short
tt_on()
{
        short   result;

        if (lcp_tv != NO)
                return 0;

        hs_posXY(POS_TOP_LIVING_ROOM,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return -1;

        gameTick(2);
        li_lool();
        lcp_tv = YES;
        p_sftvc();
        return 0;
}

/* tt_off: same walk, clear the flag, redraw the antenna in the
   off (static-line) state.  Note: no SFX_TV_CLICK on off in the 1985
   binary -- preserved verbatim.
   addr: tt_off() */

short
tt_off()
{
        short   result;

        if (lcp_tv == NO)
                return 0;

        hs_posXY(POS_TOP_LIVING_ROOM,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return -1;

        gameTick(2);
        li_lool();
        lcp_tv = NO;
        td_line(COLOR_white);
        return 0;
}

/* ---- Kitchen food-cabinet overlay ----------------------------------- */

/* sc_drfc: paint the food-count marker sprites in the
   4 slots of the open kitchen cabinet.  Count comes from bits 9..11 of
   door_states_and_flags (0..4 packs).
     1 item -> (50, 159)   3 items -> +(50, 151)
     2      -> +(58, 159)  4       -> +(58, 151)
   No-op when the cabinet is closed.
   addr: sc_drfc() */

void
sc_drfc()
{
        unsigned short  cabinet_content;

        if (lcp_cabO == NO)
                return;

        cabinet_content = (lcp.door_states_and_flags >> 9) & 7;
        od_draw(g_obi02, 46, 140);

        if (cabinet_content > 0) od_draw(g_obibg, 50, 159);
        if (cabinet_content > 1) od_draw(g_obibg, 58, 159);
        if (cabinet_content > 2) od_draw(g_obibg, 50, 151);
        if (cabinet_content > 3) od_draw(g_obibg, 58, 151);
}

/* ---- Water tank level bar (VDI polylines) ---------------------------- */

/* updWtLv: repaint or animate the water tank indicator
   at x=146..159, y=165..174.
     val == 0 : full redraw at current lcp_watr
     val <  0 : drain `-val` steps down, one game-tick each
     val >  0 : fill `val` steps up
   Each level is one 14px-wide horizontal polyline; drawn in colour 0x0D
   when filled and 0x0C (empty background) otherwise.  VDI operations
   go to the backbuffer so the animation isn't torn by the next 8Hz
   render.
   addr: updWtLv() */

void
updWtLv(val)
short   val;
{
        short   y;
        RECT16  rect;

        rect.x1 = 146;
        rect.x2 = 159;

        if (val == 0) {
                /* Draw filled portion (colour 0x0D). */
                y = lcp_watr;
                sc_sdtb();
                while (y != 0) {
                        rect.y1 = 174 - (y - 1);
                        rect.y2 = rect.y1;
                        vsl_color(vdihnd, vdi_colt[0xd]);
                        v_pline(vdihnd, 2, &rect.x1);
                        y = y - 1;
                }
                sc_sdtf();

                /* Draw empty portion (colour 0x0C). */
                y = lcp_watr;
                sc_sdtb();
                while (y < 10) {
                        rect.y1 = 174 - y;
                        rect.y2 = rect.y1;
                        vsl_color(vdihnd, vdi_colt[0xc]);
                        v_pline(vdihnd, 2, &rect.x1);
                        y = y + 1;
                }
                sc_sdtf();
                return;
        }

        if (val < 0) {
                /* Drain -val steps. */
                while (lcp_watr != 0 && val != 0) {
                        rect.y1 = 174 - (lcp_watr - 1);
                        rect.y2 = rect.y1;
                        sc_sdtb();
                        vsl_color(vdihnd, vdi_colt[0xc]);
                        v_pline(vdihnd, 2, &rect.x1);
                        sc_sdtf();
                        gameTick(4);
                        lcp_watr = lcp_watr - 1;
                        val = val + 1;
                }
                return;
        }

        /* Fill val steps (capped at 10). */
        while (val != 0 && lcp_watr < 11 &&
               (lcp_watr + 1) < 11) {
                rect.y1 = 174 - lcp_watr;
                lcp_watr = lcp_watr + 1;
                rect.y2 = rect.y1;
                sc_sdtb();
                vsl_color(vdihnd, vdi_colt[0xd]);
                v_pline(vdihnd, 2, &rect.x1);
                sc_sdtf();
                val = val - 1;
        }
}
