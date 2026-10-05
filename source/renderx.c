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

/* pickClothes: loads one of the CLOTHING_COLORS shirt colour pairs into
   palette slots 1 and 2 -- a random one half the time, otherwise the
   resident's own resident.clothingColor. */

void
pickClothes()
{
        short   index;

        index = rndRng(0, 2 * CLOTHING_COLORS - 1);
        if (index > CLOTHING_COLORS - 1)
                index = resident.clothingColor;

        mainPalette[1] = shirtPrimary[index];
        mainPalette[2] = shirtSecondary[index];
        Setpalette(mainPalette);
}

/* pickSkin: the same for the SKIN_COLORS skin tones and
   resident.skinColor. */

void
pickSkin()
{
        short   index;

        index = rndRng(0, 2 * SKIN_COLORS - 1);
        if (index > SKIN_COLORS - 1)
                index = resident.skinColor;

        mainPalette[1] = skinColors[index];
        mainPalette[2] = skinColors[index];
        Setpalette(mainPalette);
}

