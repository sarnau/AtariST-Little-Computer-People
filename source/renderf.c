/*
 * renderf.c -- sc_ren8, the main 8Hz compositor.
 *
 * Structure:
 *   1. Rate-gate on 200 Hz clock: skip until >=25 ticks (~125 ms) elapsed
 *      AND at least one VBL crossed (prevents double-render race).
 *   2. Advance dog movement + wander AI (idle countdown, food-bowl
 *      sequence, random-destination pick over 9 waypoints).
 *   3. Time out long-running SFX (doorbell -> echo, flush -> refill),
 *      advance dog eating animation.
 *   4. Background copy from house buffer, mode by tx_sctm sign:
 *          <0: partial top-strip (letter typewriter panel)
 *          =0: full-screen
 *          >0: split (letter scroll region + game area)
 *   5. Iterate 8 sprite slots: promote pending -> active, draw active.
 *   6. Vsync + Setscreen page-flip.
 *   7. Play queued SFX via sf_irqp.
 *   8. Toggle compositing target between physbase and alt buffer.
 *   9. Bump ani_cnt.
 *
 * addr: sc_ren8()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "dog.h"
#include "gfx_prim.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "renderf.h"
#include "sprglobs.h"
#include "sfx_irq.h"
#include "sound.h"
#include "sprender.h"
#include "sprglobs.h"


/* Read the 200 Hz clock via GEMDOS Super mode.  Matches Ghidra's
   screen_render_8hz shape:
       saveSSP = Super(0);
       save_hz200 = _hz_200._2_2_;   // low word of _hz_200 (long)
       save_vbclock = _vbclock;
       Super(saveSSP);
   _hz_200 lives at absolute address $04BA in TOS's low-memory system
   variables (populated by the TOS timer IRQ, no game-side handler
   needed); _vbclock lives at $0462 and is bumped by TOS's VBL IRQ.
   Both must be read in supervisor mode. */

static short
rd_hz()
{
        void *  saveSSP;
        long    v;

        saveSSP = (void *) Super(0L);
        v = *((long *) 0x04BAL);
        Super(saveSSP);
        return (short) (v & 0xFFFFL);
}

static long
rd_vbc()
{
        void *  saveSSP;
        long    v;

        saveSSP = (void *) Super(0L);
        v = *((long *) 0x0462L);
        Super(saveSSP);
        return v;
}

/* dg_pkTgt: pick next dog destination from 9-entry table.
   Extracted from sc_ren8 for readability.
   dg_vis broadens random range to include food-adjacent positions
   when the dog is on-screen. */

static void
dg_pkTgt()
{
        short   base;
        short   pick;
        short   dest_position;

        base = (dg_vis == NO) ? 0 : 3;
        do {
                pick = rndRng(base, 8);
        } while (pick == dg_ltgtI);

        dest_position = g_ddipt[pick];
        hs_posXY(dest_position,
                              &g_dtx, &g_dty);
        g_dty = g_ddyot[pick] + g_dty;
        g_dtx = g_ddxot[pick] + g_dtx;

        if (dest_position == POS_BTM_STAIR_LANDING)
                dg_nrbwl = YES;

        dg_ltgtI = pick;
        dg_idlcd    = rndRng(20, 200);
}

/* sc_ren8 -> parts/sc_ren8.c (STX: 0x15138, in the sprite object ahead of lcp_hwt). */
#ifdef FAITHFUL
#include "parts/sc_ren8.c"
#endif
