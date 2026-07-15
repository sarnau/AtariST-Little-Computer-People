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

void
endless_game_loop()
{
#ifdef __ALCYON__
        gemdos(9, "IN\r\n");
        gemdos(9, "  egl.a\r\n");
#endif
        if (g_lcldd != 0) {
                house_get_position_xy(POS_TOP_STUDY_DOOR, &lcp_x, &lcp_y);
                lcp_y = lcp_y - 3;
                lcp_x = lcp_x - 10;
                lcp_enter_study_and_save(NO, NO);
        }
#ifdef __ALCYON__
        gemdos(9, "  egl.b check copyprot\r\n");
#endif

        if (copyprot_check_return != 0) {
#ifdef __ALCYON__
                gemdos(9, "  egl.c set speed\r\n");
#endif
                game_speed_counter = 5;
#ifdef __ALCYON__
                gemdos(9, "  egl.d enter tight loop\r\n");
#endif
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
   copyprot_main_check -> sprite_init_MFDBs -> (cutscene if new) ->
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
extern void     init_vdi_and_screen();

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

        /* Palette + asset loads that we do have ported.  Once the
           real init at Ghidra 0x00015546 lands, main() delegates
           into it and this block goes away. */
        {
                short   i;
                for (i = 0; i < 16; i = i + 1)
                        xbios(7, i, main_colorpalette[i]);
        }
        al_loot();
        al_lost();
        al_locs();
        lc_load();

        {
                long raw;
                raw = gemdos(0x48, 32256L);
                if (raw == 0L) { Cconws("Malloc?\r\n"); Pterm(1); }
                g_srptr = (void *) ((raw + 255L) & ~255L);
        }
        decompress_scn("house.scn", (unsigned short *) g_srptr, 16000L);
        init_vdi_and_screen();
        endless_game_loop();

        Pterm(0);
        return 0;
}
#endif
