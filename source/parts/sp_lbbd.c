/*
 * parts/sp_lbbd.c -- shared body; LCP_STX places it in the sprite object
 * (see stx_u3.c for the address).  Files under parts/ are never
 * compiled standalone.
 */
/* sp_lbhd: dilate 21-row head frame.  Same packing as sp_lbbd but:
   start with mask = 0xFFFFFFFF, shrink from bit 31 down and bit 0 up
   until the next bit hits set img pixels -- convex-hull outline plus
   1 bit slack.  Then vertical-OR merge (opposite direction to sp_lbbd).
   addr: sp_lcp_build_all_head() */


void
sp_lbbd(src, dest, height)
short *         src;            /* signed: the reads sign-extend */
short *         dest;
short           height;
{
        /* bit / mask / img are register variables (d7/d6/d5 in
           declaration order); only h and flag get frame slots. */
        register short  bit;
        register long   mask;
        register long   img;
        short           h;
        short           flag;

        for (h = 0; h < height; h++) {
                img  = 0L;
                /* The 32-bit row is assembled by four *src++ steps,
                   not from four subscripts. */
                mask  = (long) *src++;
                mask |= (long) *src++;
                mask <<= 16;
                mask |= (long) *src++ & 0xffffL;
                mask |= (long) *src++ & 0xffffL;
                flag = 0;
                for (bit = 30; bit > 0; bit--) {
                        if (flag) {
                                img |= bm32or[bit];
                                if ((mask & bm32or[bit]) == 0L)
                                        flag = 0;
                        } else if ((mask & bm32or[bit]) != 0L) {
                                img |= bm32or[bit + 1];
                                img |= bm32or[bit];
                                img |= bm32or[bit - 1];
                                flag = 1;
                        }
                }
                *dest = (img >> 16) & 0xffffL;
                dest++;
                *dest = img & 0xffffL;
                dest++;
        }
        /* Vertical dilation: OR each row into the row above. */
        dest--;
        for (h = 0; height - 1 > h; h++) {
                img = *dest | dest[-2];
                *dest = img;
                dest--;
                img = *dest | dest[-2];
                *dest = img;
                dest--;
        }
}
