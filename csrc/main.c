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
extern short    main_colorpalette[];
extern void *   g_srptr;
extern short *  g_dsb;          /* display-screen buffer (same block as g_srptr) */
extern void *   g_dscp;         /* current-row cursor into g_dsb */
extern short    vdihandle;
extern short    al_loot();      /* asset_load_objects_table */
extern short    al_lost();      /* asset_load_sprites_table */
extern void     al_locs();      /* asset_load_character_sheets */
extern void     file_load_letter_template();
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

/* Cconws (GEMDOS 0x09) -- print a null-terminated string to CON:. */
#define Cconws(s)      gemdos(0x09, s)
/* Cconin (GEMDOS 0x01) -- read a char from CON: (blocks). */
#define Cconin()       gemdos(0x01)
/* Pterm (GEMDOS 0x4c) -- terminate cleanly. */
#define Pterm(n)       gemdos(0x4c, n)
/* Bconout (BIOS 3) -- byte to CON: (device 2).  A '\x07' rings the bell. */
#define Bconout(d, c)  bios(3, d, c)
/* Setcolor (XBIOS 7) -- change one palette slot (0..15) to 12-bit RGB. */
#define Setcolor(i, c) xbios(7, i, c)

int
main(argc, argv)
int     argc;
char ** argv;
{
        (void) argc;
        (void) argv;

        Cconws("\r\nLCP (Alcyon 4.14)\r\n");

        Cconws("[0a] appl_init...\r\n");
        appl_init();

        Cconws("[0b] graf_handle...\r\n");
        {
                short wchar, hchar, wbox, hbox, gh;
                gh = graf_handle(&wchar, &hchar, &wbox, &hbox);
                vdihandle = gh;
        }

        Cconws("[0c] v_opnvwk...\r\n");
        {
                short work_in[11];
                short work_out[57];
                short i;
                for (i = 0; i < 10; i = i + 1) work_in[i] = 1;
                work_in[0] = 1;         /* default screen device */
                work_in[10] = 2;        /* raster coords */
                v_opnvwk(work_in, &vdihandle, work_out);
        }

        Cconws("[1] palette...\r\n");
        {
                short   i;
                for (i = 0; i < 16; i = i + 1)
                        xbios(7, i, main_colorpalette[i]);
        }

        Cconws("[2] load OBJECTS...\r\n");
        al_loot();

        Cconws("[3] load SPRITES...\r\n");
        al_lost();

        Cconws("[4] load BODY/PEx.LCP...\r\n");
        al_locs();

        /* NOTE: LETTER.TXT and SOUNDS.LCP are loaded ON-DEMAND by
           the action handlers (a_writl allocates g_lttx via Malloc
           then calls file_load_letter_template; sf_sl also allocates
           per-effect).  Boot-time loading writes to NULL pointers. */

        Cconws("[5] load HYBER save...\r\n");
        lc_load();

        Cconws("[5b] alloc back-screen (32K+256)...\r\n");
        {
                long raw;
                raw = gemdos(0x48, 32256L);      /* Malloc */
                /* Align to 256-byte boundary (ST screen requirement) */
                g_srptr = (void *) ((raw + 255L) & ~255L);
                if (raw == 0L) {
                        Cconws("*** Malloc failed!\r\n");
                        Cconin();
                        Pterm(1);
                }
        }

        Cconws("[6] decompress house.scn...\r\n");
        decompress_scn("house.scn", (unsigned short *) g_srptr, 16000L);

        Cconws("[7] init VDI screen...\r\n");
        /* Point g_dsb/g_dscp at the Malloc'd back-screen block so
           init_vdi_and_screen's Setscreen sees a valid buffer instead
           of NULL (which would DMA the shifter at $0 and clobber the
           exception vector table). */
        g_dsb  = (short *) g_srptr;
        g_dscp = (void  *) (g_dsb + 0x7f);
        init_vdi_and_screen();

        Cconws("[8] about to call endless_game_loop\r\n");
        Cconws("[8a] pre-call marker\r\n");
        endless_game_loop();
        Cconws("[8c] returned (shouldn't happen)\r\n");

        Pterm(0);
        return 0;
}
#endif
