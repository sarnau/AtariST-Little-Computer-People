/*
 * abathrm.c -- hygiene handlers (bathroom-sink / shower).
 * addr: a_takes(), a_brust(), a_washh()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>             /* Random() */
#include "abathrm.h"
#include "events.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"


/* a_takes: shower.  20..25 scrub/wash cycles.  HEAD_ANIM_SHOWER bobs L/R.
   addr: a_takes() */

/* a_takes -> parts/a_takes.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_takes.c"
#endif

/* a_brust: 24..35 tooth-brush cycles.  Reuses SPRITE_STUDY_DOOR_FRAME
   (id 6) as the brush overlay above the head.
   addr: a_brust() */

/* a_brust -> parts/a_brust.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_brust.c"
#endif

/* a_washh: sink + water + 4..127 random wash cycles picking
   from 3 hand-position states.  Stops water on any interruption.
   addr: a_washh() */

/* a_washh -> parts/a_washh.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_washh.c"
#endif

/* a_driwa: fill / drink a glass (carried_object
   pre-selected by the caller).  Runs the same 3-position hand-shift
   loop as a_washh but scoped to lower amplitudes (bit 0x1f
   instead of 0x7f), so it plays for ~4..35 ticks instead of ~4..127.
   The `value` argument is the SPRITE_ID of the object being carried
   (typically SPRITE_GLASS).
   addr: a_driwa() */

/* a_driwa -> parts/a_driwa.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_driwa.c"
#endif
