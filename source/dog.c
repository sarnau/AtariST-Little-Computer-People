/*
 * dog.c -- dog movement, animation, and sprite update.
 *
 * The dog is an autonomous agent: it wanders, approaches the food bowl
 * when hungry, and comes when called.  Movement runs at 8 Hz driven by
 * dg_mvAni() from the frame loop; sprite state is pushed
 * out to hardware slots 0 or 7 (behind/in-front of LCP by Y depth) via
 * sp_spud().
 */

#include "types.h"
#include "enums.h"
#include "protos.h"
#include "globals.h"
#include "sprglobs.h"
#include "sprites.h"
/* g_sedim/g_sedms are filled by sp_regs and used by sp_sprs/sp_ssco/
   sp_ss02 as well as the dog path. */

/* Place the dog at its startup spot (bottom floor near the food bowl)
   and clear the dog sprite slots with sprite id 0.  The dog becomes
   visible on the next sc_ren8 tick once dg_mvAni picks a target and
   calls sp_spud again with a walk-cycle sprite id from g_dwanf. */
void
dg_ipos()
{
        dog_x = 100;
        dog_y = 195;
        sp_spud(0, 1, NO);
}

/* dg_mvAni -> parts/dg_mvAni.c. */

/* sp_spud (with sp_flih) lives in alerts.c, which shares its object:
   it pushes the dog frame into hardware slots 0 (behind) or 7
   (in-front) depending on layerPosition, mirroring horizontally via
   sp_flih if needed.  dg_init suppresses the push while the dog
   hasn't been placed in the world yet. */

