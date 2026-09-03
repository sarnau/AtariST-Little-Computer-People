/*
 * aidle.c -- short "no-walk" idle / gesture handlers.
 * addr: a_wandi(), a_peeka(), a_pacen(), a_toggt(), a_sleep()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "aidle.h"
#include "events.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"

/* addr: a_wandi() */
/* a_wandi -> parts/a_wandi.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_wandi.c"
#endif

/* addr: a_peeka() */
/* a_peeka -> parts/a_peeka.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_peeka.c"
#endif

/* addr: a_pacen() */
/* a_pacen -> parts/a_pacen.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_pacen.c"
#endif

/* a_toggt -> parts/a_toggt.c (STX: 0xdece object, 0x13bb2, immediately before tt_on). */
#ifdef FAITHFUL
#include "parts/a_toggt.c"
#endif

/* value == -1 is the copy-protection punishment path (sleep forever);
   the resident first walks to the current floor's center Y before lying down.
   addr: a_sleep() */
/* a_sleep -> parts/a_sleep.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_sleep.c"
#endif
