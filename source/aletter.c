/*
 * aletter.c -- ACTION_WRITE_LETTER + typewriter helpers.
 * Procedurally-assembled letter: date, salutation, 2..4 paragraphs
 * from 4 topic sections (3 lines x 4 alternates, biased by mood),
 * sign-off (g_ltg[4]), signature.  Word-wraps at 40 cols (0x27).
 * addr: a_writl(), lt_tysa(), lt_tyca()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include <stdio.h>              /* sprintf */
#include "adoors.h"
#include "aleisure.h"
#include "alerts.h"
#include "aletter.h"
#include "globals.h"
#include "letload.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"


/* a_writl: walk, malloc letter buffer, assemble body from shuffled
   template sections (2..4 paragraphs, each 3 lines picked from 4
   alternates via section_id * 96 + mood offset), free, walk out.
   addr: a_writl() */

/* a_writl -> parts/a_writl.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/a_writl.c"
#endif

/* lt_tysa: word-wrapped string typer.
   val: leading-space indent (< 0 always; > 0 only if prev line mid).
   Returns last char emitted.
   addr: lt_tysa() */

/* lt_tysa -> parts/lt_tysa.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/lt_tysa.c"
#endif


/* lt_tyca: emit one char.  CR (< space) scrolls the pane; else plays a
   random click, blits via prCh, swaps in the g_ltcwt width sprite
   per buffer position.
   addr: lt_tyca() */

/* lt_tyca -> parts/lt_tyca.c (STX orders the 0xdece object by
   function, not by file; FAITHFUL includes it back here). */
#ifdef FAITHFUL
#include "parts/lt_tyca.c"
#endif
