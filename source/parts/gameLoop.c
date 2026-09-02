/*
 * parts/gameLoop.c -- shared body; LCP_ORG links it in main.c,
 * LCP_STX in the 0xdece object (0x5c76, in the 0x400c object between lc_load and chk_actT).  Files under parts/
 * are never compiled standalone.
 */
/* gameLoop -- verified against Ghidra `endless_game_loop`.
   main() calls it as the final step (Ghidra step 40), matching the
   Ghidra decompile's structure and control flow one-for-one. */

#include <osbind.h>              /* Cconws, Cconin, Pterm, Xbtimer, ... */


void
gameLoop()
{
#ifndef FAITHFUL
        /* STX's frame is 54 bytes: 50 bytes of declared locals the
           body never reads. */
        short   unused[25];
#endif

        if (g_lcldd != 0) {
                hs_posXY(POS_TOP_STUDY_DOOR, &lcp_x, &lcp_y);
#ifdef FAITHFUL
                lcp_y = lcp_y - 3;
                lcp_x = lcp_x - 10;
#else
                lcp_y -= 3;
                lcp_x -= 10;
#endif
                lcp_std(NO, NO);
        }
#ifdef FAITHFUL
        if (cprot_r != 0) {
                g_spdc = 5;
                for (;;) {
                        gameTick(0);
                        chk_actT();
                }
        }

        for (;;)
                a_sleep(-1);
#else
        /* STX guards the other way round -- the sleep loop is the
           fall-through -- and every loop is `while (1)`, which emits
           the branch-to-condition shape `for (;;)` does not. */
        if (cprot_r == 0)
                while (1)
                        a_sleep(-1);

        g_spdc = 5;
        while (1) {
                gameTick(0);
                chk_actT();
        }
#endif
}
