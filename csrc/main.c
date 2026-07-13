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
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    lcp_x;
extern short    lcp_y;
extern short    lcp_loaded;
extern short    copyprot_check_return;
extern short    game_speed_counter;
extern void     game_tick_and_animate();
extern void     house_get_position_xy();
extern void     check_for_any_action_triggers();/* ai.c      */
extern void     game_tick_and_animate();
extern void     check_for_any_action_triggers();
extern void     a_sleep();
extern void     lcp_enter_study_and_save();

#define POS_TOP_STUDY_DOOR      7       /* HOUSE_POS index */

/* endless_game_loop: entry point from crt0 after all initialisation.
   Never returns.  The dual-loop shape (tick+AI vs sleep) is preserved
   verbatim from the 1985 binary; the copy-protection state is set by
   the loader.

   addr: endless_game_loop() */

void
endless_game_loop()
{
        if (lcp_loaded != 0) {
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
