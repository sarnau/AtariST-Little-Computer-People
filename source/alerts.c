/*
 * alerts.c -- GEM form_alert wrappers for fatal errors.
 * On host, form_alert is a no-op returning 1, so we exit instead of
 * busy-looping.
 * addr: er_nomem(), er_write()
 */

#include "types.h"
#include <osbind.h>
#include "alerts.h"

#ifdef HOST
#include <stdlib.h>             /* exit */
#include <stdio.h>              /* fprintf */
#endif

/* er_nomem lives in THIS object only in LCP_ORG; the STX revision
   puts it at the end of its 0x400c object (see stx_u1.c).  The body
   is shared via parts/ so neither configuration duplicates it. */
#ifdef FAITHFUL
#include "parts/er_nomem.c"
#endif

/* Single-shot RETRY alert; caller is expected to retry the file op.
   addr: er_write() */
void
er_write()
{
#ifdef HOST
        fprintf(stderr,
                "WARN: Unable to write to disk.\n");
#else
        form_alert(0, "[1][Unable to write.|Check disk.][RETRY]");
#endif
}

/* ---- STX-revision grouping --------------------------------------
   In LCP_STX.PRG, sp_spud and sp_flih follow er_write in this
   object (sp_spud reaches sp_flih with a bsr).  The FAITHFUL twins
   live in dog.c / sprites.c.  The STX sp_spud also splits the tail
   into two successive if/else pairs (mask pair first, then image
   pair) instead of LCP_ORG's single combined one. */
#ifndef FAITHFUL

#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "sprglobs.h"
#include "dog.h"
#include "sprites.h"
/* NOT tables.h: the STX build reads rev_tab as plain (signed) short
   -- no clr.w zero-extension before the table loads. */
extern short    rev_tab[];

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

#endif  /* !FAITHFUL */
