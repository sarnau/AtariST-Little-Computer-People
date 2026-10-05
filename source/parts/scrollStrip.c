/*
 * parts/scrollStrip.c -- included by stx_u3.c; never compiled on its own.
 */
/* scrollStrip: scroll the letter's text strip up by two scan lines when the
   typewriter wraps.  Each of 13 two-line (320-byte) blocks is copied
   onto the block above it, then lines 24 and 25 are refilled with the
   paper colour. */

void
scrollStrip()
{
        /* Declaration order and the unused short must stay: both set
           the stack-frame layout, which must match the original. */
        char *  src_ptr;
        char *  dest_ptr;
        short   unused;
        short   row;

        /* The source pointer is biased once before the loop and both
           pointers step in place after the copy, as in the original. */
        src_ptr  = (char *) g_dscp + 320;
        dest_ptr = (char *) g_dscp;
        for (row = 0; row < 13; row++) {
                copyBlocks32(src_ptr, dest_ptr, 10);
                src_ptr  += 320;
                dest_ptr += 320;
        }
        paperRow(g_dscp, 24);
        paperRow(g_dscp, 25);
}
