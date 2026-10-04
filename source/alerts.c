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

/* er_nomem and er_write live in parts/ and are included by the unity
   units (stx_u1.c, stx_u2.c) where the original's layout puts them. */

/* sp_spud and sp_flih belong to the same object as er_write in the
   original, so they are compiled here.  sp_spud's tail is deliberately
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
   released, dg_init) leaves the dog hidden.  Otherwise both slots get
   the frame's size, the dog's position (the frame's top is 17 lines
   above dog_y) and the mask, but only one slot gets the image: the
   FRONT slot when layer_p is 1, the BACK slot otherwise, so the dog is
   drawn in front of or behind the resident.  With flipH2 set the
   frame is first mirrored into g_dfimb/g_dfmab (15 lines, 2 words)
   and those buffers are used instead. */
void
sp_spud(g_seid, layer_p, flipH2)
short   g_seid;
short   layer_p;
BOOL16  flipH2;
{
        g_seaim[HW_SLOT_DOG_BACK] = NULL;
        g_seaim[HW_SLOT_DOG_FRONT] = NULL;

        if (g_seid < 0 || dg_init != NO)
                return;

        if (flipH2 != NO) {
                sp_flih(g_sedim[g_seid],
                                       (unsigned short *) g_dfimb,
                                       15, 2);
                sp_flih(g_sedms[g_seid],
                                       (unsigned short *) g_dfmab,
                                       15, 2);
        }

        g_seach[HW_SLOT_DOG_BACK] = g_sedeh[SPRITE_DOG_LAY_DOWN];
        g_seach[HW_SLOT_DOG_FRONT] = g_sedeh[SPRITE_DOG_LAY_DOWN];
        g_seacw[HW_SLOT_DOG_BACK]  = g_sedew[SPRITE_DOG_LAY_DOWN];
        g_seacw[HW_SLOT_DOG_FRONT]  = g_sedew[SPRITE_DOG_LAY_DOWN];
        g_sepex[HW_SLOT_DOG_BACK] = dog_x;
        g_sepex[HW_SLOT_DOG_FRONT] = dog_x;
        g_sepey[HW_SLOT_DOG_BACK] = dog_y - 17;
        g_sepey[HW_SLOT_DOG_FRONT] = dog_y - 17;

        if (flipH2 == NO) {
                g_seams[HW_SLOT_DOG_BACK] = g_sedms[g_seid];
                g_seams[HW_SLOT_DOG_FRONT] = g_sedms[g_seid];
        } else {
                g_seams[HW_SLOT_DOG_BACK] = g_dfmab;
                g_seams[HW_SLOT_DOG_FRONT] = g_dfmab;
        }
        if (flipH2 == NO) {
                if (layer_p == 1)
                        g_seaim[HW_SLOT_DOG_FRONT] = g_sedim[g_seid];
                else
                        g_seaim[HW_SLOT_DOG_BACK] = g_sedim[g_seid];
        } else {
                if (layer_p == 1)
                        g_seaim[HW_SLOT_DOG_FRONT] = g_dfimb;
                else
                        g_seaim[HW_SLOT_DOG_BACK] = g_dfimb;
        }
}

/* Mirror a 4-plane sprite image horizontally.  For each of pixH lines,
   the line's wdWidth 16-pixel groups (four plane words each) are
   copied in reverse group order, and every word is bit-reversed one
   byte at a time through rev_tab with the two bytes swapped.  Writes
   pixH * wdWidth * 4 words to dest. */
void
sp_flih(source, dest, pixH, wdWidth)
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
                                hi = rev_tab[v & 0xff] << 8;
                                *dest = rev_tab[(v >> 8) & 0xff] | hi;
                                dest++;
                        }
                }
                source += wdWidth << 2;
        }
}

