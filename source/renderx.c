/*
 * renderx.c -- palette, TV, screen-scroll, and prCh.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>

#ifdef HOST

#include "hostgem.h"

#else

#include <vdibind.h>

#endif
#include "obdefs1.h"
#include "protos.h"
#include "globals.h"

/* pa_cloc: pick random/configured CLOTHING_COLOR_ID (0..15),
   load prim/sec colours to palette slots 1,2. Overshoot falls back
   to lcp.clothing_color. */

void
pa_cloc()
{
        short   index;

        index = rndRng(0, 0x1f);
        if (index > 0xf)
                index = lcp.clothing_color;

        main_pal[1] = g_clcop[index];
        main_pal[2] = g_clcos[index];
        Setpalette(main_pal);
}

/* pa_skic: same as pa_cloc but 8-entry skin table. */

void
pa_skic()
{
        short   index;

        index = rndRng(0, 0xf);
        if (index > 7)
                index = lcp.skin_color;

        main_pal[1] = skin_pal[index];
        main_pal[2] = skin_pal[index];
        Setpalette(main_pal);
}

/* lcp_upal: refresh sickness tint at palette slot 6.
   ST_PEACH (0x743) healthy, ST_SICK_GREEN (0x363) sick.
   Called from sim.c (recovery), health.c (onset), lc_load (HYBER restore). */


/* td_line: draw 5-line rabbit-ear antenna on TV.
   Diagonal-up-right from (44..48, 51..49) to (44..48, 57..55).
   Colour: COLOR_white when off, random when on (static effect).
   Lives in parts/td_line.c. */

/* td_nois lives in parts/td_nois.c. */

/* sc_sctd lives in parts/sc_sctd.c. */

/* prCh: render one char via VDI.
   Sets logbase to backbuffer, MD_TRANS overlay via v_gtext, restores state.
   Setscreen (void*)-1 for phys/rez means "leave unchanged".
   Lives in parts/prCh.c. */

/* rp_anim lives in parts/rp_anim.c. */

/* strPr: paint NUL-terminated string at (x,y) via prCh, 8px/char advance
   (8x8 system font used by status strip / game menu).
   Lives in parts/strPr.c. */
