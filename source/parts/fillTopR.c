/*
 * Included by stx_u1.c; never compiled on its own.
 */

/* Paint the top panel of the drawing buffer: g_dscp is set to
   dsb_stor rounded up to a 512-byte boundary (initVdi later makes it
   the logical screen), then rows 0..max_y-2 are filled -- with the
   letter-paper colour (sc_firw) for a short panel, the two-plane
   stripe (sc_firs) for one of 70 rows or more -- and row max_y-1 is a
   black separator (sc_firb).  27 rows for the letter and game menu,
   77 for the minigames (mg_stp). */
void
fillTopR(max_y)
short   max_y;
{
        short   y;

        /* Align the buffer up to a 512-byte boundary. */
        g_dscp = (void *) dsb_stor;
        g_dscp = (void *) (((long) g_dscp + 512L) & ~511L);

        for (y = 0; y < max_y - 1; y++) {
                if (max_y < 70)
                        sc_firw(g_dscp, y);
                else
                        sc_firs(g_dscp, y);
        }
        sc_firb(g_dscp, max_y - 1);
}
