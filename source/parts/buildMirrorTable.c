/* Builds the 8-bit bit-reversal table at boot instead of shipping it
   as data. Three register variables in this declaration order and one
   frame local for the walking pointer, as in the original. initMirror
   (parts/initMirror.c) must sit immediately before it. */
void
buildMirrorTable()
{
        register short  val;
        register short  bit;
        register short  acc;
        short *         p;

        p = mirrorTable;
        for (val = 0; val < 256; val++) {
                acc = 0;
                for (bit = 0; bit < 8; bit++) {
                        if (val & mirrorSrcBit[bit])
                                acc |= mirrorDstBit[bit];
                }
                *p = acc;
                p++;
        }
}
