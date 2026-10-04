/*
 * parts/sp_lbhd.c -- included by stx_u3.c; never compiled on its own.
 */
/* sp_lbhd: build a head frame's mask.  Same packing as sp_lbbd, but
   start with mask = 0xFFFFFFFF and shrink it from bit 31 down and from
   bit 0 up until the next bit hits set img pixels -- the outline plus
   1 bit of slack.  Then a vertical-OR merge (in the opposite direction
   to sp_lbbd's). */

void
sp_lbhd(src, dest, height)
short *         src;
short *         dest;
short           height;
{
        /* bit / img / mask are register data variables and dp a
           register address variable; registers are assigned in
           declaration order, so keep the order.  Only h lives in the
           stack frame. */
        register short  bit;
        register long   img;
        register long   mask;
        register short *dp;
        short           h;

        dp = dest;
        for (h = 0; h < height; h++) {
                mask = -1L;
                img  = (long) *src++;
                img |= (long) *src++;
                img <<= 16;
                img |= (long) *src++ & 0xffffL;
                img |= (long) *src++ & 0xffffL;
                for (bit = 31; bit > 0; bit--) {
                        if ((img & bm32or[bit - 1]) != 0L)
                                break;
                        mask &= bm32and[bit];
                }
                for (bit = 0; bit < 31; bit++) {
                        if ((img & bm32or[bit + 1]) != 0L)
                                break;
                        mask &= bm32and[bit];
                }
                *dest = (mask >> 16) & 0xffffL;
                dest++;
                *dest = mask & 0xffffL;
                dest++;
        }
        /* Vertical dilation: OR each row into the row BELOW, through
           the register pointer. */
        for (h = 0; height - 1 > h; h++) {
                mask = *dp | dp[2];
                *dp++ = mask;
                mask = *dp | dp[2];
                *dp++ = mask;
        }
}
