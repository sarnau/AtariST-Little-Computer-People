/*
 * stx_u4.c -- unity unit for the sound object that immediately
 * precedes stx_u2's.
 *
 * sf_irqp calls sf_so as a same-object call, so the two share an
 * object; lt_sets (in stx_u2's object) calls sf_sele as an external,
 * so this is NOT that object.  Function order, which must not change:
 *     sgPlay < sf_irqp < sf_sl < sf_sele < sf_so
 */


/* Headers first: they emit no code, so the object layout is
   unaffected, but the parts/ body below needs them in scope. */
#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "alerts.h"
#include "globals.h"
#include "midi_seq.h"
#include "save.h"
#include "sound.h"
#include "sfx_irq.h"

#include "dat_u4.c"


#include "parts/sgPlay.c"    /* first of the object */
#include "sfx_irq.c"         /* sf_irqp */
#include "sound.c"           /* sf_sl, sf_sele, sf_so */

