/*
 * adoors.c -- door / cabinet / fridge / dresser open/close helpers.
 *
 * These are the small "close it" companions to the larger walk-and-
 * interact handlers.  They assume the resident is already at the
 * correct HOUSE_POS -- callers walk first, then call these to play the
 * animation + toggle the runtime flag + emit the SFX.
 *
 * addr: a_clotd(), a_clocd(),
 *       a_opecf(), a_opcfc(),
 *       a_opecd(), a_watat()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "adoors.h"
#include "globals.h"
#include "random.h"
#include "render.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"

/* a_clotd: 2-frame close animation.
   addr: a_clotd() */

/* a_clotd -> parts/a_clotd.c (STX: 0xdece object, 0x10556, immediately after a_uset). */
#ifdef FAITHFUL
#include "parts/a_clotd.c"
#endif

/* a_clocd: 2-frame close animation.
   addr: a_clocd() */

/* a_clocd -> parts/a_clocd.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_clocd.c"
#endif

/* STX: a_gesff sits here (0xebf8), just before a_opecf -- the default
   build includes it from stx_u2.c in STX order; FAITHFUL keeps it in
   afood.c. */

/* a_opecf: open, look inside, close.  Both SFX are
   SFX_DOOR_OPEN in the original -- preserved verbatim; whether the
   1985 source meant SFX_DOOR_CLOSE at the tail is a judgement call.
   addr: a_opecf() */

/* a_opecf -> parts/a_opecf.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_opecf.c"
#endif

/* a_opcfc: sequential open animation used by
   the write-letter and tidy-house flows.  Note that the original always
   ends with lcp_flcO = NO -- there's no "open" branch
   here; the cabinet is toggled elsewhere by walk_to_and_turn().
   addr: a_opcfc() */

/* a_opcfc -> parts/a_opcfc.c (STX: 0xdece object, 0x11d9a, immediately after a_plaag). */
#ifdef FAITHFUL
#include "parts/a_opcfc.c"
#endif

/* a_opecd: dual-mode open (value=0) / close (value=1)
   drawer with 2-frame sprite animation.
   addr: a_opecd() */

/* a_opecd -> parts/a_opecd.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_opecd.c"
#endif

/* a_watat: filing-cabinet interaction helper -- opens
   the cabinet if closed, or reaches into it if already open, then
   nervously shifts facing direction 10 times.
   addr: a_watat() */

/* a_watat -> parts/a_watat.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_watat.c"
#endif
