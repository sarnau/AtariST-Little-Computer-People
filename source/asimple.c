/*
 * asimple.c -- short idle / gesture actions.
 * addr: a_wakfa(), a_hello(), a_yawas(), a_nodh(), a_petd(), a_calld()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "actions.h"
#include "asimple.h"
#include "events.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"

/* addr: a_wakfa() */
/* a_wakfa -> parts/a_wakfa.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_wakfa.c"
#endif

/* addr: a_hello() */
/* a_hello -> parts/a_hello.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_hello.c"
#endif

/* STX places the four SFX wrappers immediately after a_hello in this
   object (a_hello reaches each with a bsr); the default build includes
   them from stx_u2.c in STX order, FAITHFUL from sound.c. */

/* addr: a_yawas() */
/* a_yawas -> parts/a_yawas.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_yawas.c"
#endif

/* addr: a_nodh() */
/* a_nodh -> parts/a_nodh.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_nodh.c"
#endif

/* addr: a_petd() */
/* a_petd -> parts/a_petd.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_petd.c"
#endif

/* addr: a_calld() */
/* a_calld -> parts/a_calld.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_calld.c"
#endif
