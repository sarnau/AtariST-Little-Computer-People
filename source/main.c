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

/* gameLoop -- verified against Ghidra `endless_game_loop`.
   main() calls it as the final step (Ghidra step 40), matching the
   Ghidra decompile's structure and control flow one-for-one. */

#include <osbind.h>              /* Cconws, Cconin, Pterm, Xbtimer, ... */


void
gameLoop()
{
        if (g_lcldd != 0) {
                hs_posXY(POS_TOP_STUDY_DOOR, &lcp_x, &lcp_y);
                lcp_y = lcp_y - 3;
                lcp_x = lcp_x - 10;
                lcp_std(NO, NO);
        }
        if (cprot_r != 0) {
                g_spdc = 5;
                for (;;) {
                        gameTick(0);
                        chk_actT();
                }
        }

        for (;;)
                a_sleep(-1);
}

#ifndef HOST
/* _stksize -- consulted by Alcyon's gemstart.o at boot to decide the
   memory model.
     +N     keep N bytes for stack+heap, return rest to OS
     -1     keep ALL memory (INCOMPATIBLE WITH GEMDOS Malloc, AES, VDI)
     -N     return N bytes to OS, keep the rest
   The 1985 game uses GEMDOS Malloc heavily (screen buffer, song
   buffer, letter template, per-SFX blocks) which needs the OS free
   pool -- so we keep 64 KB for our stack + C-heap and hand the rest
   (~440 KB) back to GEMDOS for Malloc to draw from. */
long _stksize = 65536L;

/* main -- C entry point for the Alcyon build (target only).
   Ported line-by-line from Ghidra 0x15546; see the per-step comments
   in the function body.  Excluded from the host build so tests can
   supply their own main(). */


/* Init dependencies -- Alcyon-renamed short names (see namemap.md).
   Original names in comments for cross-reference. */
/* main -- Ghidra 0x00015546.  See there for the faithful init
   sequence: mq_intim -> aes_init -> conterm clear ->
   Dsetpath("data") -> vdi_init -> stpScrB ->
   initBRev -> cntSong -> lcp_load ->
   show_title_screen_enter_name_and_date -> house.scn open+decompress ->
   fillTopR(27) -> cl_drini ->
   fLoad("body.lcp") -> lcp_crnd (if new) ->
   fLoad(pex_lcp) -> sprite_lcp_build_all -> ldObj/sprites
   -> soundeffects_load -> dog_init_position -> updWtLv
   -> screen_set_draw_to_backbuffer -> draw water pipe + doors +
   food-bowl objects -> screen_draw_food_cabinet ->
   daily_rs -> palette_apply_clothing_colors ->
   cp_main -> sp_imfs -> (cutscene if new) ->
   gameLoop.

   All of those steps are ported.  Every line in main() corresponds
   to an original Ghidra call; the few port-specific additions are
   marked inline. */


/* Object-draw chain (Ghidra main 0x15546, after unScn).
   Every door/cabinet in HOUSE.SCN has a placeholder rectangle in the
   pre-compressed art; the real init paints the correct (open or
   closed) object over each rectangle.  Skipping the chain leaves the
   placeholders visible as horizontal streaks in the affected rows. */
#include <vdibind.h>            /* vsl_color, v_pline, v_clsvwk, ... */


/* Alcyon gemlib entry points (see gemstart.o + gem.a).
   Prototypes match gembind.h / vdibind.h shape.  Declared here as
   K&R externs (empty parens) so cp68 doesn't try to typecheck them. */
#include <gembind.h>              /* appl_init, appl_exit, ... */

/* main -- ported line-by-line from Ghidra 0x15546.
   Every call below matches the Ghidra decompile in structure and
   order.  Where an Alcyon-safe short name replaces the Ghidra long
   name, the mapping is shown as a comment on the call.  Missing
   pieces (mq_intim, cntSong, init_build_bit_revert_
   table) are ported as verifiable stubs in init.c. */


int
main(argc, argv)
int     argc;
char ** argv;
{
        short   fhandle;

        (void) argc;
        (void) argv;

        /* Ghidra step 1  */  mq_intim();
        /* Ghidra step 2  */  aes_init();
        /* step 3  */  ct_clrB();
        /* step 4  */  vdi_init();
        /* step 5  */  stpScrB();
        /* step 6  */  initBRev();
        /* step 7  */  cntSong();
        /* NOTE: the ROM has NO Dsetpath -- data files live beside
           LCP.PRG -- and it never calls initBM: bm32or/bm32and stay
           zero at runtime (the sprite dilation degenerates; the ROM
           shipped that way, and initBM @0xcda0 is dead code). */
        /* Ghidra step 9  */  g_lcldd = lc_load();       /* lcp_load */
        /* Ghidra step 10 */  st_titl();                 /* show_title_screen_enter_name_and_date */

        /* Ghidra steps 11-13: open + decompress house.scn.
           Port's unScn folds Ghidra's inline fOpen +
           malloc + read + decompress into one call. */
        unScn("house.scn", (unsigned short *) g_srptr, 16000L);

        /* Ghidra step 14 */  fillTopR(27);
        /* Ghidra step 15 */  cl_drini();                /* clock_draw_initial */

        /* Ghidra steps 16-19: fLoad body.lcp, optional
           lcp_crnd, fLoad PEx.LCP.  Port's al_locs wraps
           both loads; lcp_crnd must run first for new games
           so character_sprite_id is set before al_locs builds the
           PEx filename. */
        if (g_lcldd == 0)
                lcp_crnd();
        al_locs();                                      /* body.lcp + PEx.LCP */

        /* Ghidra step 20 */  sp_lbal();                /* sprite_lcp_build_all */

        /* Ghidra steps 21-24: ldObj + parse, ldSpr +
           parse.  Port's al_loot / al_lost wrap load + parse; sp_reglp
           runs the second-pass sprite registration that Ghidra inlines. */
        al_loot();
        al_lost();
        sp_reglp();

        /* Ghidra step 25 */  sf_sl();                  /* soundeffects_load */
        /* Ghidra step 26 */  dg_ipos();                /* dog_init_position */
        /* step 28 */  updWtLv(0);

        /* Ghidra steps 29-31: water pipe polyline (147..158, 175). */
        sc_sdtb();                                      /* screen_set_draw_to_backbuffer */
        vsl_color(vdihnd, vdi_colt[0xb]);
        {
                short r[4];
                r[0] = 147; r[1] = 175; r[2] = 158; r[3] = 175;
                v_pline(vdihnd, 2, r);
        }
        sc_sdtf();                                      /* screen_set_draw_to_frontbuffer */

        /* step 32: door / cabinet draws through the od_* frame slots. */
        od_draw(lcp_cabO == NO ? od_cbcl : od_cbo2, 46, 140);
        od_draw(lcp_frdO == NO ? od_frcl : od_fro2, 294, 151);
        od_draw(lcp_drsO == NO ? od_drcl : od_dro2, 97, 115);
        od_draw(lcp_clsO == NO ? od_clcl : od_clo2, 75, 87);
        od_draw(studyDrO == NO ? od_stcl : od_sto2, 178, 23);
        od_draw(lcp_toiO == NO ? od_tocl : od_too2, 187, 87);
        od_draw(lcp_flcO == NO ? od_ficl : od_fio2, 258, 47);

        /* Ghidra step 33: dog bowl.  Ghidra writes three `if (status
           == BOWL_X)` branches; array indexing on g_obdea produces the
           same object per state. */
        od_draw(g_obdea[lcp_bwlS], 8, 190);

        /* Ghidra step 34 */  sc_drfc();                /* screen_draw_food_cabinet */
        /* Ghidra step 35 */  daily_rs();
        /* Ghidra step 36 */  pa_cloc();                /* palette_apply_clothing_colors */
        /* Ghidra step 37 */  cprot_r = cp_main();  /* copyprot_main_check */
        /* Ghidra step 38 */  sp_imfs();                /* sprite_init_MFDBs */
        /* Ghidra step 39 */
        if (g_lcldd == 0)
                cs_mvIn();        /* cutscene_new_lcp_move_in */

        /* step 40: gameLoop -- then Pterm, as the ROM does. */
        gameLoop();
        gemdos(0x4C, 0);
        return 0;
}

/* ct_clrB (ROM 0x42e, same object as main -- called via bsr): clear
   bits 0..2 of the TOS system variable `conterm` at 0x484 under
   GEMDOS Super. */

void
ct_clrB()
{
        void *  saveSSP;
        char *  conterm_ptr;

        /* ROM 0x42e: bare gemdos(0x20) shapes -- no 0L padding here,
           and the conterm byte is read via a signed char. */
        saveSSP = (void *) gemdos(0x20, 0L);
        conterm_ptr = (char *) 0x484L;
        *conterm_ptr = *conterm_ptr & 0xF8;
        gemdos(0x20, saveSSP);
}

#endif
