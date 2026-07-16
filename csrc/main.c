/*
 * main.c -- top-level game loop.
 *
 * endless_game_loop() is called once from the CRT startup after title
 * screen, palette, and save-file setup.  Two modes:
 *
 *   copy protection passed -> tight (tick + AI) loop forever
 *   copy protection failed -> sleep(-1) loop forever
 *
 * The sleep loop is the 1985 anti-piracy behaviour: a cracked binary
 * would appear to run but never actually simulate.  The check itself
 * (copyprot_check_return) is set once during startup and is treated as
 * an ordinary flag from here.
 *
 * addr: endless_game_loop()
 */

#include "types.h"
#include "enums.h"

/* endless_game_loop definition (gated back IN for both host and
   Alcyon builds).  main() does NOT call it yet -- we're bisecting
   what part of the archive triggers the pre-main crash by seeing
   whether merely LINKING with these transitive refs breaks the
   canary. */

extern long     gemdos();       /* used for Cconws markers */
extern short    lcp_x;
extern short    lcp_y;
extern short    g_lcldd;
extern short    copyprot_check_return;
extern short    game_speed_counter;
extern void     game_tick_and_animate();
extern void     house_get_position_xy();
extern void     check_for_any_action_triggers();
extern void     a_sleep();
extern void     lcp_enter_study_and_save();

#define POS_TOP_STUDY_DOOR      7

extern void     check_for_any_action_triggers();

void
endless_game_loop()
{
        if (g_lcldd != 0) {
                house_get_position_xy(POS_TOP_STUDY_DOOR, &lcp_x, &lcp_y);
                lcp_y = lcp_y - 3;
                lcp_x = lcp_x - 10;
                lcp_enter_study_and_save(NO, NO);
        }
        if (copyprot_check_return != 0) {
                game_speed_counter = 5;
                for (;;) {
                        game_tick_and_animate(0);
                        check_for_any_action_triggers();
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
   before jumping into endless_game_loop(); those pre-init hooks are
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
   sequence: mq_intim -> aes_vdi_jnit -> conterm clear ->
   Dsetpath("data") -> vdi_init -> setup_screen_buffer ->
   init_build_bit_revert_table -> count_songs -> lcp_load ->
   show_title_screen_enter_name_and_date -> house.scn open+decompress ->
   fill_top_rect_with_background(27) -> cl_drini ->
   file_load("body.lcp") -> lcp_create_random (if new) ->
   file_load(pex_lcp) -> sprite_lcp_build_all -> load_objects/sprites
   -> soundeffects_load -> dog_init_position -> update_water_level_bar
   -> screen_set_draw_to_backbuffer -> draw water pipe + doors +
   food-bowl objects -> screen_draw_food_cabinet ->
   daily_reset_action_flags -> palette_apply_clothing_colors ->
   cp_main -> sp_imfs -> (cutscene if new) ->
   endless_game_loop.

   Almost none of those subroutines are ported yet; this stub only
   exercises the parts we do have so LCP.PRG at least builds and
   launches for smoke tests.  Every added line here should either
   correspond to an original Ghidra call, or be marked as diagnostic
   scaffolding to remove. */

extern short    main_colorpalette[];
extern void *   g_srptr;
extern short    vdihandle;
extern short    al_loot();      /* asset_load_objects_table */
extern short    al_lost();      /* asset_load_sprites_table */
extern void     al_locs();      /* asset_load_character_sheets */
extern void     sf_sl();        /* soundeffects_load */
extern short    lc_load();      /* lcp_load */
extern void     decompress_scn();
extern void     setup_screen_buffer();  /* Ghidra 0x16576 */
extern void     sp_imfs();              /* Ghidra sprite_init_MFDBs */
extern void     sp_lbal();              /* Ghidra sprite_lcp_build_all */
extern void     init_bitmask_tables();  /* fills bm32or/and */
extern short    cp_main();  /* Ghidra (stubbed to return 1) */
extern void     pa_cloc();  /* Ghidra palette_apply_clothing_colors */
extern void     sp_reglp();     /* sprload.c: dog sprite pointer registration */
extern void     dg_ipos();  /* dog.c */
extern void *   save_physbase;

/* Object-draw chain (Ghidra main 0x15546, after decompress_scn).
   Every door/cabinet in HOUSE.SCN has a placeholder rectangle in the
   pre-compressed art; the real init paints the correct (open or
   closed) object over each rectangle.  Skipping the chain leaves the
   placeholders visible as horizontal streaks in the affected rows. */
extern void     fill_top_rect_with_background();       /* render.c */
extern void     od_draw();                              /* render.c */
extern void     sc_drfc();                              /* render.c: food cabinet */
extern void     update_water_level_bar();               /* render.c */
extern void     sc_sdtb();                              /* gfx_prim.c */
extern void     sc_sdtf();                              /* gfx_prim.c */
extern void     vsl_color();                            /* VDI */
extern void     v_pline();                              /* VDI */
extern short    _vdi_color_table[];
extern void     cl_drini();                   /* init.c */
extern void     lcp_create_random();                    /* init.c */
extern void     cutscene_new_lcp_move_in_stub();        /* init.c */
extern void     st_titl();                              /* init.c */
extern void     draw_hud_top_strip();                   /* init.c */
extern void     daily_reset_action_flags();             /* calendar.c */
extern short    lcp_cabinet_open;
extern short    lcp_front_door_open;
extern short    lcp_dresser_open;
extern short    lcp_closet_door_open;
extern short    lcp_study_door_open;
extern short    lcp_toilet_door_open;
extern short    lcp_filing_cabinet_open;
extern short    lcp_dog_bowl_status;
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
extern short    appl_init();
extern void     v_clsvwk();
extern void     appl_exit();
extern void     aes_vdi_jnit();                 /* Ghidra 0x167aa */
extern void     vdi_init();                     /* Ghidra 0x16680 */

#define Cconws(s)       gemdos(0x09, s)
#define Cconin()        gemdos(0x01)
#define Pterm(n)        gemdos(0x4c, n)

/* main -- ported line-by-line from Ghidra 0x15546.
   Every call below matches the Ghidra decompile in structure and
   order.  Where an Alcyon-safe short name replaces the Ghidra long
   name, the mapping is shown as a comment on the call.  Missing
   pieces (mq_intim, count_songs, init_build_bit_revert_
   table) are ported as verifiable stubs in init.c. */

extern void     mq_intim();          /* init.c stub    */
extern void     count_songs();                  /* init.c         */
extern void     init_build_bit_revert_table();  /* init.c wrapper */
extern void     conterm_clear_bits012();        /* declared below */
extern void     sf_sl();                        /* soundeffects_load */
extern void     cutscene_new_lcp_move_in_stub();/* init.c         */

int
main(argc, argv)
int     argc;
char ** argv;
{
        short   fhandle;

        (void) argc;
        (void) argv;

        /* Ghidra step 1  */  /* mq_intim(); -- disabled during bisect */
        /* Ghidra step 2  */  aes_vdi_jnit();
        /* Ghidra step 3  */  /* conterm_clear_bits012(); -- disabled */
        /* Ghidra step 4: Dsetpath -- disabled */
        /* Ghidra step 5  */  vdi_init();
        /* Ghidra step 6  */  setup_screen_buffer();
        /* Ghidra step 7  */  init_build_bit_revert_table();
        /* Ghidra step 8  */  /* count_songs(); -- disabled */
        /* Ghidra step 9  */  g_lcldd = lc_load();       /* lcp_load */
        /* Ghidra step 10 */  st_titl();                 /* show_title_screen_enter_name_and_date */

        /* Ghidra steps 11-13: open + decompress house.scn.
           Port's decompress_scn folds Ghidra's inline file_open +
           malloc + read + decompress into one call. */
        decompress_scn("house.scn", (unsigned short *) g_srptr, 16000L);

        /* Ghidra step 14 */  fill_top_rect_with_background(27);
        /* Ghidra step 15 */  cl_drini();                /* clock_draw_initial */

        /* Ghidra steps 16-19: file_load body.lcp, optional
           lcp_create_random, file_load PEx.LCP.  Port's al_locs wraps
           both loads; lcp_create_random must run first for new games
           so character_sprite_id is set before al_locs builds the
           PEx filename. */
        if (g_lcldd == 0)
                lcp_create_random();
        al_locs();                                      /* body.lcp + PEx.LCP */

        /* Ghidra step 20 */  sp_lbal();                /* sprite_lcp_build_all */

        /* Ghidra steps 21-24: load_objects + parse, load_sprites +
           parse.  Port's al_loot / al_lost wrap load + parse; sp_reglp
           runs the second-pass sprite registration that Ghidra inlines. */
        al_loot();
        al_lost();
        sp_reglp();

        /* Ghidra step 25 */  /* sf_sl(); -- disabled */
        /* Ghidra step 26 */  dg_ipos();                /* dog_init_position */
        /* Ghidra step 27 */
        /* if (g_lcldd == 0) sp_spud(0, 1, NO); -- disabled */

        /* Ghidra step 28 */  update_water_level_bar(0);

        /* Ghidra steps 29-31: water pipe polyline (147..158, 175). */
        sc_sdtb();                                      /* screen_set_draw_to_backbuffer */
        vsl_color(vdihandle, _vdi_color_table[0xb]);
        {
                short r[4];
                r[0] = 147; r[1] = 175; r[2] = 158; r[3] = 175;
                v_pline(vdihandle, 2, r);
        }
        sc_sdtf();                                      /* screen_set_draw_to_frontbuffer */

        /* Ghidra step 32: door / cabinet object_draws.  Ghidra writes
           seven `if (state == NO) A else B` pairs; port uses ternaries. */
        od_draw(lcp_cabinet_open        == NO ? g_obicc : g_obi02, 46,  140);
        od_draw(lcp_front_door_open     == NO ? g_obidf : g_obi06, 294, 151);
        od_draw(lcp_dresser_open        == NO ? g_obi11 : g_obi12, 97,  115);
        od_draw(lcp_closet_door_open    == NO ? g_obidc : g_obi04, 75,  87);
        od_draw(lcp_study_door_open     == NO ? g_obids : g_obi08, 178, 23);
        od_draw(lcp_toilet_door_open    == NO ? g_obidt : g_obi10, 187, 87);
        od_draw(lcp_filing_cabinet_open == NO ? g_obifc : g_obi14, 258, 47);

        /* Ghidra step 33: dog bowl.  Ghidra writes three `if (status
           == BOWL_X)` branches; array indexing on g_obdea produces the
           same object per state. */
        od_draw(g_obdea[lcp_dog_bowl_status], 8, 190);

        /* Ghidra step 34 */  sc_drfc();                /* screen_draw_food_cabinet */
        /* Ghidra step 35 */  daily_reset_action_flags();
        /* Ghidra step 36 */  pa_cloc();                /* palette_apply_clothing_colors */
        /* Ghidra step 37 */  copyprot_check_return = cp_main();  /* copyprot_main_check */
        /* Ghidra step 38 */  sp_imfs();                /* sprite_init_MFDBs */
        /* Ghidra step 39 */
        if (g_lcldd == 0)
                cutscene_new_lcp_move_in_stub();        /* cutscene_new_lcp_move_in */

        /* Ghidra step 40: endless_game_loop, never returns. */
        endless_game_loop();

        (void) fhandle;                                 /* unused local */

        Pterm(0);
        return 0;
}

/* conterm_clear_bits012 (Ghidra 0x15546:0x??): clear bits 0..2 of
   the TOS system variable `conterm` at 0x484.  Must run in supervisor
   mode; port uses GEMDOS Super to elevate, mask conterm, restore. */

void
conterm_clear_bits012()
{
        void *          saveSSP;
        unsigned char * conterm_ptr;

        saveSSP = (void *) gemdos(GEMDOS_Super, 0L);
        conterm_ptr = (unsigned char *) 0x484L;
        *conterm_ptr = *conterm_ptr & 0xF8;
        gemdos(GEMDOS_Super, (long) saveSSP);
}

#endif
