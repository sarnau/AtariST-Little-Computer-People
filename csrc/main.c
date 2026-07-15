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
   memory model.  A value of -1 means "keep ALL memory for this
   process; return nothing to the OS".  Alcyon's gemstart references
   this as an extern; if we don't define it, the linker resolves it
   to a garbage BSS value and gemstart's memory-management calcs
   crash before main() is ever reached (symptom: busy-bee cursor
   forever). */
long _stksize = -1L;

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

        /* Boot marker: turn background (palette slot 0) red + beep.
           Confirms main() ran (gemstart+libc CRT reached us).  Absent
           if the link order is wrong (gemstart.o must be first). */
        Setcolor(0, 0x700);
        Bconout(2, 7);
        Cconws("\r\nLittle Computer People (Alcyon 4.14 port)\r\n");
        Cconws("Entering endless_game_loop...\r\n");

        /* Runtime pre-init the 1985 CRT did before endless_game_loop:
              title screen, palette load, save-file setup, VDI init.
           None ported yet -- expect the game loop to hang or crash
           on the first VDI/asset call.  When it does, add the
           relevant init step here. */
        endless_game_loop();

        /* endless_game_loop() never returns; if it did, exit cleanly. */
        Pterm(0);
        return 0;
}
#endif
