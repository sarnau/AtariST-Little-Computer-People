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


/* LCP_STX inlines all three of the helpers the port used to keep
   here -- the two Super-mode clock reads and the dog target picker --
   directly into sc_ren8 (parts/sc_ren8.c), so they are gone. */

/* sc_ren8 -> parts/sc_ren8.c (STX: 0x15138, in the sprite object ahead of lcp_hwt). */
