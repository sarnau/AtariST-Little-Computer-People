/*
 * parts/sc_sctd.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x16d5a, in the 0x148fe object ahead of sc_firw). Files
 * under parts/ are never compiled standalone.
 */
/* sc_sctd: 1-row block scroll on top text strip (letter typewriter wrap).
   Copies 13 rows of 40 words each downward, blanks top two rows to white.
   addr: sc_sctd() */

void
sc_sctd()
{
        /* STX's frame is -16: src, dest, an unused short, then row. */
        char *  src_ptr;
        char *  dest_ptr;
        short   unused;
        short   row;

        /* STX biases the source pointer once before the loop and
           steps both pointers in place after the copy. */
        src_ptr  = (char *) g_dscp + 320;
        dest_ptr = (char *) g_dscp;
        for (row = 0; row < 13; row++) {
                blkcp32(src_ptr, dest_ptr, 10);
                src_ptr  += 320;
                dest_ptr += 320;
        }
        sc_firw(g_dscp, 24);
        sc_firw(g_dscp, 25);
}
