/* Scroll the letter's text strip up by two scan lines when the
   typewriter wraps.  Each of 13 two-line (320-byte) blocks is copied
   onto the block above it, then lines 24 and 25 are refilled with the
   paper colour. */

void
scrollStrip()
{
        /* Declaration order and the unused short must stay: both set
           the stack-frame layout, which must match the original. */
        char *  srcPtr;
        char *  destPtr;
        short   unused;
        short   row;

        /* The source pointer is biased once before the loop and both
           pointers step in place after the copy, as in the original. */
        srcPtr  = (char *) stripBuf + 320;
        destPtr = (char *) stripBuf;
        for (row = 0; row < 13; row++) {
                copyBlocks32(srcPtr, destPtr, 10);
                srcPtr  += 320;
                destPtr += 320;
        }
        paperRow(stripBuf, 24);
        paperRow(stripBuf, 25);
}
