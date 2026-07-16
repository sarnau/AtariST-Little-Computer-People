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
   sequence: midi_seq_init_timer -> aes_vdi_jnit -> conterm clear ->
   Dsetpath("data") -> vdi_init -> setup_screen_buffer ->
   init_build_bit_revert_table -> count_songs -> lcp_load ->
   show_title_screen_enter_name_and_date -> house.scn open+decompress ->
   fill_top_rect_with_background(27) -> clock_draw_initial ->
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


/* Alcyon gemlib entry points (see gemstart.o + gem.a).
   Prototypes match gembind.h / vdibind.h shape.  Declared here as
   K&R externs (empty parens) so cp68 doesn't try to typecheck them. */
extern short    appl_init();
extern short    graf_handle();
extern void     v_opnvwk();
extern void     v_clsvwk();
extern void     appl_exit();

#define Cconws(s)       gemdos(0x09, s)
#define Cconin()        gemdos(0x01)
#define Pterm(n)        gemdos(0x4c, n)

int
main(argc, argv)
int     argc;
char ** argv;
{
        (void) argc;
        (void) argv;

        Cconws("\r\nLCP (Alcyon 4.14)\r\n");

        appl_init();
        {
                short wchar, hchar, wbox, hbox, gh;
                gh = graf_handle(&wchar, &hchar, &wbox, &hbox);
                vdihandle = gh;
        }
        {
                short work_in[11];
                short work_out[57];
                short i;
                for (i = 0; i < 10; i = i + 1) work_in[i] = 1;
                work_in[0] = 1;
                work_in[10] = 2;
                v_opnvwk(work_in, &vdihandle, work_out);
        }

        /* aes_vdi_jnit tail (Ghidra 0x??): the real init loads the
           full palette in one Setpalette and snapshots the TOS
           physbase so sc_ren8 can page-flip between it and the alt
           buffer.  Both matter for correct on-screen output --
           without the Physbase snapshot, sc_ren8's page-flip lands
           on garbage and the house appears wrapped/shifted. */
        xbios(6, (long) main_colorpalette);             /* XBIOS Setpalette */
        save_physbase = (void *) xbios(2);              /* XBIOS Physbase */

        setup_screen_buffer();          /* Ghidra 0x16576 */
        init_bitmask_tables();
        al_loot();
        al_lost();
        sp_reglp();                     /* populate g_sedim[] / g_sedms[] */
        al_locs();                      /* body.lcp + PEx.LCP */
        sp_lbal();                      /* Ghidra sprite_lcp_build_all */
        lc_load();                      /* Ghidra lcp_load */
        dg_ipos();            /* Ghidra: place dog at (100,195) */
        decompress_scn("house.scn", (unsigned short *) g_srptr, 16000L);

        /* Post-decompress paint chain from Ghidra main (0x15546).
           Draws every door/cabinet at its current lcp state on top of
           the HOUSE.SCN background so the pre-drawn placeholders stop
           bleeding through as horizontal streaks.  fill_top_rect_with
           _background(27) paints the top status strip (rows 0..26)
           with the striped/black separator pattern; g_dsb is set to
           (short*)(g_srptr - 254) in setup_screen_buffer so the +0x7f
           word offset resolves to row 0 of the offscreen house. */
        fill_top_rect_with_background(27);

        od_draw(lcp_cabinet_open       == NO ? g_obicc : g_obi02, 46,  140);
        od_draw(lcp_front_door_open    == NO ? g_obidf : g_obi06, 294, 151);
        od_draw(lcp_dresser_open       == NO ? g_obi11 : g_obi12, 97,  115);
        od_draw(lcp_closet_door_open   == NO ? g_obidc : g_obi04, 75,  87);
        od_draw(lcp_study_door_open    == NO ? g_obids : g_obi08, 178, 23);
        od_draw(lcp_toilet_door_open   == NO ? g_obidt : g_obi10, 187, 87);
        od_draw(lcp_filing_cabinet_open == NO ? g_obifc : g_obi14, 258, 47);
        sc_drfc();                              /* food cabinet contents */

        pa_cloc();                      /* palette_apply_clothing_colors */
        copyprot_check_return = cp_main();
        sp_imfs();                      /* Ghidra sprite_init_MFDBs */
        endless_game_loop();

        Pterm(0);
        return 0;
}
#endif
