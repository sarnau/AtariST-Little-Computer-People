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
   sequence: midi_seq_init_timer -> aes_vdi_jnit -> conterm clear ->
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

int
main(argc, argv)
int     argc;
char ** argv;
{
        (void) argc;
        (void) argv;

        Cconws("\r\nLCP (Alcyon 4.14)\r\n");

        /* --- Ghidra step 2: aes_vdi_jnit (0x167aa) ------------------ */
        aes_vdi_jnit();

        /* --- Ghidra step 5: vdi_init (0x16680) ---------------------- */
        vdi_init();

        /* --- Ghidra step 6: setup_screen_buffer --------------------- */
        setup_screen_buffer();
        init_bitmask_tables();          /* step 7: bit_revert_table */

        /* --- Ghidra step 9: lcp_load MUST come before decompress ----- */
        g_lcldd = lc_load();

        /* --- Ghidra step 10: title screen + name/date/time prompt ---- */
        st_titl();

        /* --- Ghidra steps 11-13: house.scn open + decompress --------- */
        decompress_scn("house.scn", (unsigned short *) g_srptr, 16000L);

        /* --- Ghidra step 14: clear top strip of the OFFscreen house -- */
        fill_top_rect_with_background(27);

        /* --- Ghidra step 15: clock face background dot + hands ------- */
        cl_drini();

        /* --- Ghidra steps 16-19: body.lcp + optional lcp_create_random
               + PEx.LCP.  al_locs wraps body+PEx together, so we sequence
               lcp_create_random between them by splitting.  Simpler
               path here: run lcp_create_random FIRST if no save file,
               so al_locs picks up the correct character_sprite_id when
               it builds the PEx.LCP filename. */
        if (g_lcldd == 0)
                lcp_create_random();

        al_locs();                      /* body.lcp + PEx.LCP */

        /* --- Ghidra step 20: sprite_lcp_build_all -------------------- */
        sp_lbal();

        /* --- Ghidra steps 21-24: object + sprite tables -------------- */
        al_loot();
        al_lost();
        sp_reglp();                     /* populate g_sedim/g_sedms */

        /* --- Ghidra step 25: soundeffects_load (TODO, not yet ported)  */

        /* --- Ghidra step 26: dog_init_position (position only) ------- */
        dg_ipos();

        /* --- Ghidra step 28: water tank level bar ------------------- */
        update_water_level_bar(0);

        /* --- Ghidra steps 29-31: water pipe polyline (147..158,175) - */
        sc_sdtb();
        vsl_color(vdihandle, _vdi_color_table[0xb]);
        {
                short pts[4];
                pts[0] = 147; pts[1] = 175;
                pts[2] = 158; pts[3] = 175;
                v_pline(vdihandle, 2, pts);
        }
        sc_sdtf();

        /* --- Ghidra step 32: door / cabinet object_draws ------------- */
        od_draw(lcp_cabinet_open       == NO ? g_obicc : g_obi02, 46,  140);
        od_draw(lcp_front_door_open    == NO ? g_obidf : g_obi06, 294, 151);
        od_draw(lcp_dresser_open       == NO ? g_obi11 : g_obi12, 97,  115);
        od_draw(lcp_closet_door_open   == NO ? g_obidc : g_obi04, 75,  87);
        od_draw(lcp_study_door_open    == NO ? g_obids : g_obi08, 178, 23);
        od_draw(lcp_toilet_door_open   == NO ? g_obidt : g_obi10, 187, 87);
        od_draw(lcp_filing_cabinet_open == NO ? g_obifc : g_obi14, 258, 47);

        /* --- Ghidra step 33: dog bowl object based on bowl status ----
           g_obdea[] maps BOWL_EMPTY/HALF/FULL to their object ids. */
        od_draw(g_obdea[lcp_dog_bowl_status], 8, 190);

        /* --- Ghidra step 34: food cabinet contents ------------------- */
        sc_drfc();

        /* --- Ghidra step 35: reset once-per-day flags ---------------- */
        daily_reset_action_flags();

        /* --- Ghidra step 36: apply LCP clothing palette entries ------ */
        pa_cloc();

        /* --- Ghidra step 37: copy protection ------------------------- */
        copyprot_check_return = cp_main();

        /* --- Ghidra step 38: sprite MFDB init ------------------------ */
        sp_imfs();

        /* --- Ghidra step 39: cutscene (new game only) ---------------- */
        if (g_lcldd == 0)
                cutscene_new_lcp_move_in_stub();

        /* --- Ghidra step 40: endless game loop ----------------------- */
        endless_game_loop();

        Pterm(0);
        return 0;
}
#endif
