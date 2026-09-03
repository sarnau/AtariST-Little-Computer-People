/*
 * parts/fillTopR.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x686c -- in LCP_STX this is the 0x400c object, not 0xdece).
 * Files under parts/ are never compiled standalone.
 */

void
fillTopR(max_y)
short   max_y;
{
        short   y;

        /* STX aligns the buffer up to a 512-byte boundary instead. */
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
