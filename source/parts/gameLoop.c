/*
 * parts/gameLoop.c -- included by stx_u1.c; never compiled on its own.
 * main()'s last step: the endless game loop.
 */

#include <osbind.h>              /* Cconws, Cconin, Pterm, Xbtimer, ... */


void
gameLoop()
{
        /* 50 bytes of locals the body never reads.  They must stay:
           removing them changes the compiled code. */
        short   unused[25];

        if (g_lcldd != 0) {
                hs_posXY(POS_TOP_STUDY_DOOR, &lcp_x, &lcp_y);
                lcp_y -= 3;
                lcp_x -= 10;
                lcp_std(NO, NO);
        }
        /* The sleep loop is the fall-through of the guard, and every
           loop is `while (1)`: `for (;;)` compiles differently. */
        if (cprot_r == 0)
                while (1)
                        a_sleep(SLEEP_RANDOM);

        g_spdc = 5;
        while (1) {
                gameTick(0);
                chk_actT();
        }
}
