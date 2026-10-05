/*
 * stx_u4.c -- unity unit for the sound object that immediately
 * precedes stx_u2's.
 *
 * startSfx calls stopSfx as a same-object call, so the two share an
 * object; typeKeySound (in stx_u2's object) calls sfxSelect as an external,
 * so this is NOT that object.  Function order, which must not change:
 *     playSongFile < startSfx < loadSounds < sfxSelect < stopSfx
 */


/* Headers first: they emit no code, so the object layout is
   unaffected, but the parts/ body below needs them in scope. */
#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "protos.h"
#include "globals.h"

#include "dat_sound.c"


#include "parts/playSongFile.c"    /* first of the object */
#include "sfx_irq.c"         /* startSfx */
#include "sound.c"           /* loadSounds, sfxSelect, stopSfx */

