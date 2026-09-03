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
        /* The other revision parks the dog on the "unused" sprite id -1; the
           STX revision passes id 0. */
        sp_spud(0, 1, NO);
}

/* dg_mvAni -> parts/dg_mvAni.c (STX: 0x412c). */

/* sp_spud: push the dog frame into hardware slots 0
   (behind) or 7 (in-front) depending on layerPosition; mirror horizontally
   via sp_flih if needed.  dg_init suppresses the
   push while the dog hasn't been placed in the world yet.

   The STX revision groups it (and
   sp_flih) with the alerts object -- see the twin in alerts.c.
   addr: sp_spud() */

