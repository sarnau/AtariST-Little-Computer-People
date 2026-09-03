/*
 * parts/sp_lbhd.c -- shared body; LCP_STX places it in the sprite object
 * (see stx_u3.c for the address).  Files under parts/ are never
 * compiled standalone.
 */
/* sp_lbal: dispatch 98 body + 66 head frames through sp_lbbd/sp_lbhd.
   addr: sp_lcp_build_all() */

void
sp_lbhd(src, dest, height)
short *         src;
short *         dest;
short           height;
{
        /* bit / img / mask are register data variables (d7/d6/d5) and
           dp a register ADDRESS variable (a5); only h gets a frame
           slot. */
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
