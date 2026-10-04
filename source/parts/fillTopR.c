/*
 * Included by stx_u1.c; never compiled on its own.
 */

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
