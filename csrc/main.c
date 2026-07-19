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
 * addr: gameLoop()
 */

#include "types.h"
#include "enums.h"
#include <osbind.h>

/* gameLoop definition (gated back IN for both host and
   Alcyon builds).  main() does NOT call it yet -- we're bisecting
   what part of the archive triggers the pre-main crash by seeing
   whether merely LINKING with these transitive refs breaks the
   canary. */

#include <osbind.h>              /* Cconws, Cconin, Pterm, Xbtimer, ... */
extern short    lcp_x;
extern short    lcp_y;
extern short    g_lcldd;
extern short    cprot_r;
extern short    g_spdc;
extern void     gameTick();
extern void     hs_posXY();
extern void     chk_actT();
extern void     a_sleep();
extern void     lcp_std();

#define POS_TOP_STUDY_DOOR      7

extern void     chk_actT();

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

/* main -- C entry point for the Alcyon build (target only).  The 1985
   startup path did title screen + palette + save-file setup here
   before jumping into gameLoop(); those pre-init hooks are
   still TODO.  Meanwhile this drives just enough of the runtime to
   verify the .PRG actually executes: prints a marker string, waits
   for a key, then exits cleanly.  Excluded from the host build so
   tests can supply their own main(). */

extern long gemdos();
extern long bios();
extern long xbios();

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

   Almost none of those subroutines are ported yet; this stub only
   exercises the parts we do have so LCP.PRG at least builds and
   launches for smoke tests.  Every added line here should either
   correspond to an original Ghidra call, or be marked as diagnostic
   scaffolding to remove. */

extern short    main_pal[];
extern void *   g_srptr;
extern short    vdihnd;
extern short    al_loot();      /* asset_load_objects_table */
extern short    al_lost();      /* asset_load_sprites_table */
extern void     al_locs();      /* asset_load_character_sheets */
extern void     sf_sl();        /* soundeffects_load */
extern short    lc_load();      /* lcp_load */
extern void     unScn();
extern void     stpScrB();  /* Ghidra 0x16576 */
extern void     sp_imfs();              /* Ghidra sprite_init_MFDBs */
extern void     sp_lbal();              /* Ghidra sprite_lcp_build_all */
extern void     initBM();  /* fills bm32or/and */
extern short    cp_main();  /* Ghidra (stubbed to return 1) */
extern void     pa_cloc();  /* Ghidra palette_apply_clothing_colors */
extern void     sp_reglp();     /* sprload.c: dog sprite pointer registration */
extern void     dg_ipos();  /* dog.c */
extern void *   sv_phb;

/* Object-draw chain (Ghidra main 0x15546, after unScn).
   Every door/cabinet in HOUSE.SCN has a placeholder rectangle in the
   pre-compressed art; the real init paints the correct (open or
   closed) object over each rectangle.  Skipping the chain leaves the
   placeholders visible as horizontal streaks in the affected rows. */
extern void     fillTopR();       /* render.c */
extern void     od_draw();                              /* render.c */
extern void     sc_drfc();                              /* render.c: food cabinet */
extern void     updWtLv();               /* render.c */
extern void     sc_sdtb();                              /* gfx_prim.c */
extern void     sc_sdtf();                              /* gfx_prim.c */
extern short    vsl_color();                            /* VDI */
extern void     v_pline();                              /* VDI */
extern short    vdi_colt[];
extern void     cl_drini();                   /* init.c */
extern void     lcp_crnd();                    /* init.c */
extern void     cs_mvIn();        /* init.c */
extern void     st_titl();                              /* init.c */
extern void     daily_rs();             /* calendar.c */
extern short    lcp_cabO;
extern short    lcp_frdO;
extern short    lcp_drsO;
extern short    lcp_clsO;
extern short    studyDrO;
extern short    lcp_toiO;
extern short    lcp_flcO;
extern short    lcp_bwlS;
extern short    g_obicc, g_obi02;                       /* cabinet cl/op */
extern short    g_obidf, g_obi06;                       /* door_front */
extern short    g_obi11, g_obi12;                       /* dresser */
extern short    g_obidc, g_obi04;                       /* door_closet */
extern short    g_obids, g_obi08;                       /* door_study */
extern short    g_obidt, g_obi10;                       /* door_toilet */
extern short    g_obifc, g_obi14;                       /* filing_cabinet */
extern short    g_obdea[];                              /* dog_eating_animation frame table */


/* Alcyon gemlib entry points (see gemstart.o + gem.a).
   Prototypes match gembind.h / vdibind.h shape.  Declared here as
   K&R externs (empty parens) so cp68 doesn't try to typecheck them. */
#include <gembind.h>              /* appl_init, appl_exit, ... */
extern void     v_clsvwk();
extern void     aes_init();                 /* Ghidra 0x167aa */
extern void     vdi_init();                     /* Ghidra 0x16680 */

/* main -- ported line-by-line from Ghidra 0x15546.
   Every call below matches the Ghidra decompile in structure and
   order.  Where an Alcyon-safe short name replaces the Ghidra long
   name, the mapping is shown as a comment on the call.  Missing
   pieces (mq_intim, cntSong, init_build_bit_revert_
   table) are ported as verifiable stubs in init.c. */

extern void     mq_intim();          /* init.c stub    */
extern void     cntSong();                  /* init.c         */
extern void     initBRev();  /* init.c wrapper */
extern void     ct_clrB();        /* declared below */
extern void     sf_sl();                        /* soundeffects_load */
extern void     cs_mvIn();/* init.c         */

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
        /* Ghidra step 3  */  ct_clrB();
        /* Ghidra step 4  */  Dsetpath("data");
        /* Ghidra step 5  */  vdi_init();
        /* Ghidra step 6  */  stpScrB();
        /* Ghidra step 7  */  initBRev();
        /* Port-specific: fill bm32or/bm32and at runtime.  The ROM ships
           these as static const long[32] tables in DATA, but Alcyon C
           doesn't accept the `UL` suffix on hex constants > 0x7FFFFFFF
           so we compute them at runtime instead.  Must run before
           sp_lbal (step 20) which reads bm32or in its dilation loop. */
                              initBM();
        /* Ghidra step 8  */  cntSong();
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
        /* Ghidra step 27: clear dog sprite slots before cs_mvIn.  Ghidra
           passes `~SPRITE_UNUSED_0` (=-1) as g_seid so sp_spud's
           `if (g_seid < 0) return;` early-exit fires -- the only
           observable effect is g_seaim[0] = g_seaim[7] = NULL at the
           top.  The port had this call disabled with the wrong
           constant (0 instead of -1) -- verified at ROM 0x15846-1584e
           via /read_memory:
               3f 3c 00 01    move.w #1,  -(sp)   ; layer_p = 1
               3f 3c ff ff    move.w #-1, -(sp)   ; g_seid  = -1
               4e b9 ...      jsr sp_spud         ; flipH already 0 on stack */
        if (g_lcldd == 0)
                sp_spud(-1, 1, NO);

        /* Ghidra step 28 */  updWtLv(0);

        /* Ghidra steps 29-31: water pipe polyline (147..158, 175). */
        sc_sdtb();                                      /* screen_set_draw_to_backbuffer */
        vsl_color(vdihnd, vdi_colt[0xb]);
        {
                short r[4];
                r[0] = 147; r[1] = 175; r[2] = 158; r[3] = 175;
                v_pline(vdihnd, 2, r);
        }
        sc_sdtf();                                      /* screen_set_draw_to_frontbuffer */

        /* Ghidra step 32: door / cabinet object_draws.  Ghidra writes
           seven `if (state == NO) A else B` pairs; port uses ternaries. */
        od_draw(lcp_cabO        == NO ? g_obicc : g_obi02, 46,  140);
        od_draw(lcp_frdO     == NO ? g_obidf : g_obi06, 294, 151);
        od_draw(lcp_drsO        == NO ? g_obi11 : g_obi12, 97,  115);
        od_draw(lcp_clsO    == NO ? g_obidc : g_obi04, 75,  87);
        od_draw(studyDrO     == NO ? g_obids : g_obi08, 178, 23);
        od_draw(lcp_toiO    == NO ? g_obidt : g_obi10, 187, 87);
        od_draw(lcp_flcO == NO ? g_obifc : g_obi14, 258, 47);

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

        /* Ghidra step 40: gameLoop, never returns. */
        gameLoop();

        (void) fhandle;                                 /* unused local */

        Pterm(0);
        return 0;
}

/* ct_clrB (Ghidra 0x15546:0x??): clear bits 0..2 of
   the TOS system variable `conterm` at 0x484.  Must run in supervisor
   mode; port uses GEMDOS Super to elevate, mask conterm, restore. */

void
ct_clrB()
{
        void *          saveSSP;
        unsigned char * conterm_ptr;

        saveSSP = (void *) Super(0L);
        conterm_ptr = (unsigned char *) 0x484L;
        *conterm_ptr = *conterm_ptr & 0xF8;
        Super(saveSSP);
}

#endif
