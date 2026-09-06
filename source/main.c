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
 *
 * addr: Ghidra `endless_game_loop` (called from the tail of main at
 * ROM 0x15546).
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

/* gameLoop -> parts/gameLoop.c (STX: 0x5c76, in the 0x400c object between lc_load and chk_actT). */

#ifndef HOST
/* No `_stksize` here.  That global is the ATARI DK gemstart's
   memory-model hook; alcyon2's GEMSTART.O -- the startup LCP_STX
   links -- has the stack size baked in and contains no reference to
   it (neither does its GEMLIB).  Defining it only put 4 dead bytes at
   the head of the data segment, ahead of psg_rot, which LCP_STX has
   at 0x180. */

/* main -- C entry point for the Alcyon build (target only).
   Ported line-by-line from Ghidra 0x15546; see the per-step comments
   in the function body.  Excluded from the host build so tests can
   supply their own main(). */


/* Init dependencies -- Alcyon-renamed short names (see namemap.md).
   Original names in comments for cross-reference. */
/* main -- Ghidra 0x00015546.  See there for the faithful init
   sequence: mq_intim -> aes_init -> conterm clear ->
   Dsetpath("data") -> vdi_init -> stpScrB ->
   initBRev -> cntSong -> lc_load ->
   show_title_screen_enter_name_and_date -> house.scn open+decompress ->
   fillTopR(27) -> cl_drini ->
   al_loal("body.lcp") -> lcp_crnd (if new) ->
   al_loal(pex_lcp) -> sprite_lcp_build_all -> ldObj/sprites
   -> soundeffects_load -> dog_init_position -> updWtLv
   -> screen_set_draw_to_backbuffer -> draw water pipe + doors +
   food-bowl objects -> screen_draw_food_cabinet ->
   daily_rs -> palette_apply_clothing_colors ->
   cp_main -> sp_imfs -> (cutscene if new) ->
   gameLoop.

   All of those steps are ported.  Every line in main() corresponds
   to an original Ghidra call; the few port-specific additions are
   marked inline. */


/* Object-draw chain (Ghidra main 0x15546, after scn_dec).
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

/* main -- ported line-by-line from Ghidra 0x15546.
   Every call below matches the Ghidra decompile in structure and
   order.  Where an Alcyon-safe short name replaces the Ghidra long
   name, the mapping is shown as a comment on the call.  Missing
   pieces (mq_intim, cntSong, init_build_bit_revert_
   table) are ported as verifiable stubs in init.c. */


/* main -> parts/main.c (STX 0x5546, in the 0x400c object between
   fr_reac and dg_ipos); stx_u1.c includes it there.  ct_clrB does not
   exist as a function in LCP_STX -- main inlines the conterm patch. */

#endif
