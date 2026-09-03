/*
 * stx_u4.c -- STX unity unit for the sound object that immediately
 * precedes the big 0xdece one.
 *
 * Evidence: sf_irqp reaches sf_so with a bsr (0xdb2e -> 0xddd8), so
 * the two share an object; lt_sets (0x1476c, inside the 0xdece
 * object) reaches sf_sele with a jsr, so this is NOT that object.
 * Order follows the byte-matched members' STX addresses:
 *     sgPlay 0xd9ea < sf_irqp 0xdafc < sf_sl 0xdcc4
 *     < sf_sele 0xdd88 < sf_so 0xddd8
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


#include "parts/sgPlay.c"    /* 0xd9ea, first of the object */
#include "sfx_irq.c"         /* sf_irqp 0xdafc */
#include "sound.c"           /* sf_sl 0xdcc4 < sf_sele < sf_so */

