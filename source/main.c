/*
 * main.c -- top-level game loop.
 *
 * gameLoop() is called once from the CRT startup after title
 * screen, palette, and save-file setup.  Two modes:
 *
 *   copy protection passed -> tight (tick + AI) loop forever
 *   copy protection failed -> sleep(-1) loop forever
 *
 * The sleep loop is the 1985 anti-piracy behaviour: a cracked binary
 * would appear to run but never actually simulate.  The check itself
 * (cprot_r) is set once during startup and is treated as
 * an ordinary flag from here.
 */

#include "types.h"
#include "enums.h"
#include <osbind.h>
#include "ai.h"
#include "aidle.h"
#include "assets.h"
#include "calendar.h"
#include "dog.h"
#include "gfx_prim.h"
#include "globals.h"
#include "init.h"
#include "main.h"
#include "movement.h"
#include "render.h"
#include "renderx.h"
#include "save.h"
#include "sound.h"
#include "sprites.h"
#include "sprload.h"
#include "stubs.h"
#include "tables.h"
#include "tick.h"
#include "tick_tables.h"

/* gameLoop -> parts/gameLoop.c. */

#ifndef HOST
/* No `_stksize` here.  That global is the ATARI DK gemstart's
   memory-model hook; alcyon2's GEMSTART.O -- the startup this program
   links -- has the stack size baked in and contains no reference to
   it (neither does its GEMLIB).  Defining it would put 4 dead bytes
   at the head of the data segment. */

/* main -- C entry point for the Alcyon build (target only).
   Excluded from the host build so tests can supply their own main().

   Init sequence: mq_intim -> aes_init -> conterm clear ->
   Dsetpath("data") -> vdi_init -> stpScrB ->
   initBRev -> cntSong -> lc_load ->
   title screen (name and date) -> house.scn open+decompress ->
   fillTopR(27) -> cl_drini ->
   al_loal("body.lcp") -> lcp_crnd (if new) ->
   al_loal(pex_lcp) -> build the LCP sprites -> ldObj/sprites
   -> load sound effects -> dog start position -> updWtLv
   -> draw to the back buffer -> draw water pipe + doors +
   food-bowl objects -> draw the food cabinet ->
   daily_rs -> clothing colours -> cp_main -> sp_imfs ->
   (cutscene if new) -> gameLoop. */


/* Object-draw chain (after scn_dec).
   Every door/cabinet in HOUSE.SCN has a placeholder rectangle in the
   pre-compressed art; the real init paints the correct (open or
   closed) object over each rectangle.  Skipping the chain leaves the
   placeholders visible as horizontal streaks in the affected rows. */
#ifdef HOST
#include "hostgem.h"
#else
#include <vdibind.h>            /* vsl_color, v_pline, v_clsvwk, ... */
#endif


/* Alcyon gemlib entry points (see gemstart.o + gem.a).
   Prototypes match gembind.h / vdibind.h shape.  Declared here as
   K&R externs (empty parens) so cp68 doesn't try to typecheck them. */
#ifdef HOST
#include "hostgem.h"
#else
#include <gembind.h>              /* appl_init, appl_exit, ... */
#endif

/* main -> parts/main.c; stx_u1.c includes it.  main patches conterm
   inline rather than through a helper. */

#endif
