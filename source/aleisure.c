/*
 * aleisure.c -- music, fireplace, couch, exercise, and lightweight
 * house-upkeep handlers.
 * addr: a_lists(), a_playp(), a_plawr(), a_lighf(), a_socwd(),
 *       a_sitae(), a_chefd(), a_cleau(), a_tidyh(), a_opcbc()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "actions.h"
#include "adoors.h"
#include "ahouse.h"
#include "aleisure.h"
#include "asimple.h"
#include "delivery.h"
#include "events.h"
#include "globals.h"
#include "midi_seq.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "save.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"


/* a_lists -> parts/a_lists.c (STX: 0x1398c, right after drwPixel). */
#ifdef FAITHFUL
#include "parts/a_lists.c"
#endif

/* a_playp -> parts/a_playp.c (STX: 0x13a62, right after a_lists). */
#ifdef FAITHFUL
#include "parts/a_playp.c"
#endif

/* a_plawr: browse vinyl shelf, play a random .org file.  Animation is
   amplitude-reactive: poll PSG channel volumes via Giaccess and pick
   a browsing pose when any channel got louder.  Host PSG stub returns
   0 forever, so it holds the reach-right pose.
   addr: a_plawr() */

/* a_plawr -> parts/a_plawr.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_plawr.c"
#endif

/* a_lighf: firewood run from front-door pickup to the
   fireplace, then stoke animation with a random-facing shrug pattern
   and 2500..5000 tick fire-active countdown.
   addr: a_lighf() */

/* a_lighf -> parts/a_lighf.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_lighf.c"
#endif

/* a_socwd: call the dog over, sit on the couch,
   pet the dog for 30..50 ticks then crouch back off the couch.
   addr: a_socwd() */

/* a_socwd -> parts/a_socwd.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_socwd.c"
#endif

/* a_sitae: stretch arms in 4-frame cycle for a random
   number of iterations.
   addr: a_sitae() */

/* a_sitae -> parts/a_sitae.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_sitae.c"
#endif

/* a_chefd: walk to the door, open it, look outside for
   `value` ticks, then optionally close.  value is passed by the doAct
   dispatcher as 40 (see actions.c switch).
   addr: a_chefd() */

/* a_chefd -> parts/a_chefd.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_chefd.c"
#endif

/* a_tidyh: walk to filing cabinet, possibly close it.
   addr: a_tidyh() */

/* a_tidyh -> parts/a_tidyh.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_tidyh.c"
#endif

/* a_cleau: sweep all open doors/cabinets and close them.
   Order: upstairs first so downstairs animations don't collide with
   the toilet-door sprite pipeline.
   addr: a_cleau() */

/* a_cleau -> parts/a_cleau.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_cleau.c"
#endif

/* a_opcbc: 3-sprite dress-change sequence.
   Door swings open, 3-frame in-closet animation with palette swap
   (clothing/skin), door swings back.
   value=0 -> pa_cloc, value=1 -> pa_skic.
   addr: a_opcbc() */

/* a_opcbc -> parts/a_opcbc.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_opcbc.c"
#endif

/* a_opcuc: walk to study door, 3-frame open if closed, enter study.
   Chains into lcp_std; value != 0 -> do_save=YES.
   addr: a_opcuc() */

/* a_opcuc -> parts/a_opcuc.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_opcuc.c"
#endif
