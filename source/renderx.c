/*
 * renderx.c -- palette, TV, screen-scroll, and printChar.
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

/* pickClothes: pick random/configured CLOTHING_COLOR_ID (0..15),
   load prim/sec colours to palette slots 1,2. Overshoot falls back
   to lcp.clothing_color. */

void
pickClothes()
{
        short   index;

        index = rndRng(0, 0x1f);
        if (index > 0xf)
                index = lcp.clothing_color;

        main_pal[1] = g_clcop[index];
        main_pal[2] = g_clcos[index];
        Setpalette(main_pal);
}

/* pickSkin: same as pickClothes but 8-entry skin table. */

void
pickSkin()
{
        short   index;

        index = rndRng(0, 0xf);
        if (index > 7)
                index = lcp.skin_color;

        main_pal[1] = skin_pal[index];
        main_pal[2] = skin_pal[index];
        Setpalette(main_pal);
}

/* setSkinColor: refresh sickness tint at palette slot 6.
   ST_PEACH (0x743) healthy, ST_SICK_GREEN (0x363) sick.
   Called from sim.c (recovery), health.c (onset), loadSavedGame (HYBER restore). */


/* drawTvPicture: draw 5-line rabbit-ear antenna on TV.
   Diagonal-up-right from (44..48, 51..49) to (44..48, 57..55).
   Colour: COLOR_white when off, random when on (static effect).
   Lives in parts/drawTvPicture.c. */

/* tvNoise lives in parts/tvNoise.c. */

/* scrollStrip lives in parts/scrollStrip.c. */

/* printChar: render one char via VDI.
   Sets logbase to backbuffer, MD_TRANS overlay via v_gtext, restores state.
   Setscreen (void*)-1 for phys/rez means "leave unchanged".
   Lives in parts/printChar.c. */

/* animRecPlayer lives in parts/animRecPlayer.c. */

/* printString: paint NUL-terminated string at (x,y) via printChar, 8px/char advance
   (8x8 system font used by status strip / game menu).
   Lives in parts/printString.c. */
