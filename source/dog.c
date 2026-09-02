/*
 * dog.c -- dog movement, animation, and sprite update.
 *
 * The dog is an autonomous agent: it wanders, approaches the food bowl
 * when hungry, and comes when called.  Movement runs at 8 Hz driven by
 * dg_mvAni() from the frame loop; sprite state is pushed
 * out to hardware slots 0 or 7 (behind/in-front of LCP by Y depth) via
 * sp_spud().
 *
 * addr: dg_mvAni(), sp_spud()
 */

#include "types.h"
#include "enums.h"
#include "dog.h"
#include "globals.h"
#include "movement.h"
#include "sprglobs.h"
#include "sprites.h"
#include "walk.h"
/* g_sedim/g_sedms = PTR_ARRAY_0005a156/0x54016; populated by sp_reglp
   and used by sp_sprs/sp_ssco/sp_ss02 as well as the dog path. */

/* dog_init_position: Ghidra 0x??.  Place the dog at its startup spot
   (bottom floor near the food bowl) and NULL-out the two dog sprite
   slots via sp_spud(SPRITE_UNUSED_0=-1).  The dog becomes visible on
   the very next sc_ren8 tick once dg_mvAni picks a target
   and calls sp_spud again with a valid walk-cycle sprite id from
   g_dwanf. */
void
dg_ipos()
{
        dog_x = 100;
        dog_y = 195;
        /* LCP_ORG parks the dog on the "unused" sprite id -1; the
           STX revision passes id 0. */
#ifdef FAITHFUL
        sp_spud(-1, 1, NO);
#else
        sp_spud(0, 1, NO);
#endif
}

/* dg_mvAni -> parts/dg_mvAni.c (STX: 0x412c). */
#ifdef FAITHFUL
#include "parts/dg_mvAni.c"
#endif

/* sp_spud: push the dog frame into hardware slots 0
   (behind) or 7 (in-front) depending on layerPosition; mirror horizontally
   via sp_flih if needed.  dg_init suppresses the
   push while the dog hasn't been placed in the world yet.

   LCP_ORG links this here in dog.c; the STX revision groups it (and
   sp_flih) with the alerts object -- see the twin in alerts.c.
   addr: sp_spud() */

#ifdef FAITHFUL
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
                if (layer_p == 1)
                        g_seaim[HW_SLOT_DOG_FRONT] = g_sedim[g_seid];
                else
                        g_seaim[HW_SLOT_DOG_BACK] = g_sedim[g_seid];
        } else {
                g_seams[HW_SLOT_DOG_BACK] = g_dfmab;
                g_seams[HW_SLOT_DOG_FRONT] = g_dfmab;
                if (layer_p == 1)
                        g_seaim[HW_SLOT_DOG_FRONT] = g_dfimb;
                else
                        g_seaim[HW_SLOT_DOG_BACK] = g_dfimb;
        }
}
#endif  /* FAITHFUL */
