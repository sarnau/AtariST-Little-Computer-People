/*
 * alerts.c -- GEM form_alert wrappers for fatal errors.
 * On host, form_alert is a no-op returning 1, so we exit instead of
 * busy-looping.
 */

#include "types.h"
#include <osbind.h>
#include "protos.h"

#ifdef HOST
#include <stdlib.h>             /* exit */
#include <stdio.h>              /* fprintf */
#endif

/* outOfMemory and writeErrorAlert live in parts/ and are included by the unity
   units (stx_u1.c, stx_u2.c) where the original's layout puts them. */

/* setDogSprite and flipSprite belong to the same object as writeErrorAlert in the
   original, so they are compiled here.  setDogSprite's tail is deliberately
   two successive if/else pairs (mask pair first, then image pair)
   rather than one combined if/else: that is how the original is
   written. */

#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tables.h"

/* Load the dog's frame into the two dog hardware sprite slots.  Both
   slots are cleared first; a negative frame id (or the dog not yet
   released, dogHidden) leaves the dog hidden.  Otherwise both slots get
   the frame's size, the dog's position (the frame's top is 17 lines
   above dogY) and the mask, but only one slot gets the image: the
   FRONT slot when layer_p is 1, the BACK slot otherwise, so the dog is
   drawn in front of or behind the resident.  With flipH2 set the
   frame is first mirrored into dogMirImage/dogMirMask (15 lines, 2 words)
   and those buffers are used instead. */
void
setDogSprite(g_seid, layer_p, flipH2)
short   g_seid;
short   layer_p;
BOOL16  flipH2;
{
        drawnImage[HW_SLOT_DOG_BACK] = NULL;
        drawnImage[HW_SLOT_DOG_FRONT] = NULL;

        if (g_seid < 0 || dogHidden != NO)
                return;

        if (flipH2 != NO) {
                flipSprite(spriteBitmap[g_seid],
                                       (unsigned short *) dogMirImage,
                                       15, 2);
                flipSprite(spriteMask[g_seid],
                                       (unsigned short *) dogMirMask,
                                       15, 2);
        }

        drawnHeight[HW_SLOT_DOG_BACK] = spriteHeight[SPRITE_DOG_LAY_DOWN];
        drawnHeight[HW_SLOT_DOG_FRONT] = spriteHeight[SPRITE_DOG_LAY_DOWN];
        drawnWidth[HW_SLOT_DOG_BACK]  = spriteWidth[SPRITE_DOG_LAY_DOWN];
        drawnWidth[HW_SLOT_DOG_FRONT]  = spriteWidth[SPRITE_DOG_LAY_DOWN];
        pendX[HW_SLOT_DOG_BACK] = dogX;
        pendX[HW_SLOT_DOG_FRONT] = dogX;
        pendY[HW_SLOT_DOG_BACK] = dogY - 17;
        pendY[HW_SLOT_DOG_FRONT] = dogY - 17;

        if (flipH2 == NO) {
                drawnMask[HW_SLOT_DOG_BACK] = spriteMask[g_seid];
                drawnMask[HW_SLOT_DOG_FRONT] = spriteMask[g_seid];
        } else {
                drawnMask[HW_SLOT_DOG_BACK] = dogMirMask;
                drawnMask[HW_SLOT_DOG_FRONT] = dogMirMask;
        }
        if (flipH2 == NO) {
                if (layer_p == 1)
                        drawnImage[HW_SLOT_DOG_FRONT] = spriteBitmap[g_seid];
                else
                        drawnImage[HW_SLOT_DOG_BACK] = spriteBitmap[g_seid];
        } else {
                if (layer_p == 1)
                        drawnImage[HW_SLOT_DOG_FRONT] = dogMirImage;
                else
                        drawnImage[HW_SLOT_DOG_BACK] = dogMirImage;
        }
}

/* Mirror a 4-plane sprite image horizontally.  For each of pixH lines,
   the line's wdWidth 16-pixel groups (four plane words each) are
   copied in reverse group order, and every word is bit-reversed one
   byte at a time through mirrorTable with the two bytes swapped.  Writes
   pixH * wdWidth * 4 words to dest. */
void
flipSprite(source, dest, pixH, wdWidth)
unsigned short *        source;
unsigned short *        dest;
short                   pixH;
short                   wdWidth;
{
        short                   y;
        short                   x;
        short                   planeIndex;
        short                   v;
        short                   hi;
        unsigned short *        img_ptr;

        for (y = 0; y < pixH; y++) {
                for (x = 0; x < wdWidth; x++) {
                        img_ptr = source + (((wdWidth - 1) - x) << 2);
                        for (planeIndex = 0; planeIndex < 4;
                             planeIndex++) {
                                v = *img_ptr;
                                img_ptr++;
                                hi = mirrorTable[v & 0xff] << 8;
                                *dest = mirrorTable[(v >> 8) & 0xff] | hi;
                                dest++;
                        }
                }
                source += wdWidth << 2;
        }
}

