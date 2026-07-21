/*
 * render.c -- VDI palette and screen refresh.
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

/* cl_redrH: erase prev hands in white, draw new pair in grey.
   Skips when t_min hasn't advanced past cached g_cmmin.
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

/* od_draw: blit background object at (x,y) via vro_cpyfm S_ONLY.
   addr: od_draw() */


void
od_draw(g_oiidx, x, y)
short   g_oiidx;
short   x;
short   y;
{
        /* Ghidra: `object_tab_mfdb + object_index` (MFDB* pointer
           arithmetic).  An earlier port used `(char*)g_otmfd + g_oiidx`
           which treated g_oiidx as a byte offset instead of an array
           index -- MFDB is 18 bytes so all non-zero object IDs
           landed misaligned and VDI got junk width/height.  Fixed by
           indexing directly into the typed MFDB[] array. */
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
                          &g_obtmt[g_oiidx],
                          &mf_scrp);
        }
}

/* fillTopR: clear top text strip (rows 0..maxY-1).
   White fill for letter pane (maxY < 70), striped house-bg fill otherwise.
   Last row painted black as separator.
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

/* sc_sctd, td_nois, rp_anim -> renderx.c */

/* -- TV toggle -- */

/* tt_on: walk to living room, idle look-left, set flag, play click SFX.
   Returns -1 on walk failure, 0 otherwise.
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

/* tt_off: same walk, clear flag, redraw antenna in off state.
   Note: no SFX_TV_CLICK on off in the 1985 binary -- preserved verbatim.
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

/* -- Kitchen food-cabinet overlay -- */

/* sc_drfc: paint food-count markers in 4 cabinet slots.
   Count = bits 9..11 of door_states_and_flags (0..4 packs). No-op if closed.
     1 -> (50,159)  2 -> (58,159)  3 -> (50,151)  4 -> (58,151)
   addr: sc_drfc() */

void
sc_drfc()
{
        unsigned short  cabinet_content;

        if (lcp_cabO == NO)
                return;

        cabinet_content = (lcp.door_states_and_flags >> 9) & 7;
        od_draw(OBJ_CABINET_OPEN_2, 46, 140);

        if (cabinet_content > 0) od_draw(OBJ_CABINET_ITEM, 50, 159);
        if (cabinet_content > 1) od_draw(OBJ_CABINET_ITEM, 58, 159);
        if (cabinet_content > 2) od_draw(OBJ_CABINET_ITEM, 50, 151);
        if (cabinet_content > 3) od_draw(OBJ_CABINET_ITEM, 58, 151);
}

/* -- Water tank level bar (VDI polylines) -- */

/* updWtLv: repaint/animate water tank indicator at x=146..159, y=165..174.
     val == 0 : full redraw at current lcp_watr
     val <  0 : drain `-val` steps, one game-tick each
     val >  0 : fill `val` steps
   Each level = one 14px horizontal polyline; colour 0x0D filled, 0x0C empty.
   VDI ops go to backbuffer so animation isn't torn by next 8Hz render.
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
