/*
 * parts/rv_bld.c -- builds the 8-bit bit-reversal table at boot
 * instead of shipping it as data.  Three register variables in this
 * declaration order and one frame local for the walking pointer, as in
 * the original.  initBRev (parts/initBRev.c) must sit immediately
 * before it.
 *
 * Included by stx_u1.c; never compiled on its own.
 */
void
rv_bld()
{
        register short  val;
        register short  bit;
        register short  acc;
        short *         p;

        p = rev_tab;
        for (val = 0; val < 256; val++) {
                acc = 0;
                for (bit = 0; bit < 8; bit++) {
                        if (val & rv_msk[bit])
                                acc |= rv_val[bit];
                }
                *p = acc;
                p++;
        }
}
