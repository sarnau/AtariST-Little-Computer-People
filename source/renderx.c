/*
 * renderx.c -- the resident's random clothing and skin colours.
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
   to resident.clothing_color. */

void
pickClothes()
{
        short   index;

        index = rndRng(0, 0x1f);
        if (index > 0xf)
                index = resident.clothing_color;

        mainPalette[1] = shirtPrimary[index];
        mainPalette[2] = shirtSecondary[index];
        Setpalette(mainPalette);
}

/* pickSkin: same as pickClothes but 8-entry skin table. */

void
pickSkin()
{
        short   index;

        index = rndRng(0, 0xf);
        if (index > 7)
                index = resident.skin_color;

        mainPalette[1] = skinColors[index];
        mainPalette[2] = skinColors[index];
        Setpalette(mainPalette);
}

