/*
 * render.c -- VDI palette and screen refresh.
 * addr: sc_ren8(), lcp_upal()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <vdibind.h>
#include "vdiown.h"
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


#ifdef FAITHFUL
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
#endif  /* FAITHFUL -- STX groups it with cl_drini in init.c */

/* od_draw: blit background object at (x,y) through the game's own
   vro_cpy binding (ROM 0x97d0 -> 0xd8d2).
   addr: od_draw() */


void
od_draw(g_oiidx, x, y)
short   g_oiidx;
short   x;
short   y;
{
        vroCpyD(vdihnd, 3,
                /* LCP_ORG goes through the g_obtmp pointer variable;
                   the STX revision addresses the MFDB array itself. */
#ifdef FAITHFUL
                g_oiidx * 20 + (long) g_obtmp,
#else
                g_oiidx * 20 + (long) g_obtmt,
#endif
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
   addr: fillTopR() */


void
fillTopR(max_y)
short   max_y;
{
        short   y;

        g_dscp = (void *) ((long) g_dsb + 254L);        /* ROM 0x9868 */

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

/* tt_on -> parts/tt_on.c (STX: 0xdece object, 0x13bc8, immediately after a_toggt). */
#ifdef FAITHFUL
#include "parts/tt_on.c"
#endif

/* tt_off: same walk, clear flag, redraw antenna in off state.
   Note: no SFX_TV_CLICK on off in the 1985 binary -- preserved verbatim.
   addr: tt_off() */

/* tt_off -> parts/tt_off.c (STX: 0xdece object, 0x13c1e, immediately after tt_on). */
#ifdef FAITHFUL
#include "parts/tt_off.c"
#endif

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
        od_draw(od_cbo2, 46, 140);

        if (cabinet_content > 0) od_draw(od_cbit, 50, 159);
        if (cabinet_content > 1) od_draw(od_cbit, 58, 159);
        if (cabinet_content > 2) od_draw(od_cbit, 50, 151);
        if (cabinet_content > 3) od_draw(od_cbit, 58, 151);
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
        rect.y1 = 174;
        rect.x2 = 159;
        rect.y2 = 174;

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
