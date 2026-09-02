/*
 * parts/rv_bld.c -- LCP_STX builds the 8-bit bit-reversal table at
 * boot (0x680e) instead of shipping it as data.  Three register
 * variables (d7/d6/d5 in declaration order) and one frame local for
 * the walking pointer.  initBRev (parts/initBRev.c) must sit
 * immediately before it -- the call is a bsr.s.
 * Files under parts/ are never compiled standalone.
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
