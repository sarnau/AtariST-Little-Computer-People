/*
 * ahouse.c -- walk-and-interact action handlers.
 *
 * Ports for actions that walk somewhere in the house, play an
 * interaction animation with SFX, and update world state.
 *
 * addr: a_readn(), a_gioob(),
 *       a_dance(), a_drink(), a_uset(),
 *       a_wakum(), a_gotbn()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>             /* Random() */
#include "abathrm.h"
#include "adoors.h"
#include "afood.h"
#include "ahouse.h"
#include "aleisure.h"
#include "asimple.h"
#include "events.h"
#include "globals.h"
#include "health.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"


/* a_readn: armchair + TV + 200-frame reading loop.
   addr: a_readn() */

/* a_readn -> parts/a_readn.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_readn.c"
#endif

/* a_gioob: undress and lie down, or reverse.
   addr: a_gioob() */

/* a_gioob -> parts/a_gioob.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_gioob.c"
#endif

/* a_dance: turn on the record player if needed, then step-shift
   until the song ends or the event queue interrupts.
   addr: a_dance() */

/* a_dance -> parts/a_dance.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_dance.c"
#endif

/* a_drink: sink -> glass -> tap -> drink -> reset thirst.
   addr: a_drink() */

/* a_drink -> parts/a_drink.c (STX: 0xdece object, 0x121d6, immediately before updWtLv). */
#ifdef FAITHFUL
#include "parts/a_drink.c"
#endif

/* a_uset: 3-sprite door animation, sit + flush + refill.
   addr: a_uset() */

/* a_uset -> parts/a_uset.c (STX: 0xdece object, 0x101be, immediately before a_clotd). */
#ifdef FAITHFUL
#include "parts/a_uset.c"
#endif

/* a_wakum: scheduled morning routine.
   addr: a_wakum() */

/* a_wakum -> parts/a_wakum.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_wakum.c"
#endif

/* a_gotbn: scheduled bedtime routine.
   addr: a_gotbn() */

/* a_gotbn -> parts/a_gotbn.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_gotbn.c"
#endif

/* a_getd: pure head-anim routine.  Turns the head to face
   a canonical resting direction, then oscillates the vertical tilt bit
   four times (undressing / dressing motion communicated via head bob).
   No walking, no world state change.
   addr: a_getd() */

/* a_getd -> parts/a_getd.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_getd.c"
#endif

/* li_lool / li_loor: the two 4-tick "stand-and-
   look" gestures used by the TV toggle, record player, and post-action
   idle transitions.  The 1985 code sets FACING_RIGHT in both -- the
   "left" / "right" naming refers to which head-frame direction the
   animation actually plays via g_hatas, not the body
   facing.  Preserved verbatim.
   addr: li_lool(), li_loor() */

/* li_loor -> parts/li_loor.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/li_loor.c"
#endif

/* li_lool -> parts/li_lool.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/li_lool.c"
#endif
