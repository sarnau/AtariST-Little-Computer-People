/* Build a body frame's mask.  Each 32-bit row is widened by
   one pixel on either side of every run of set pixels, then merged
   vertically with its neighbouring row. */


void
maskBody(src, dest, height)
short *         src;            /* signed: the reads sign-extend */
short *         dest;
short           height;
{
        /* bit / mask / img are register variables; registers are
           assigned in declaration order, so keep the order.  Only h
           and flag live in the stack frame. */
        register short  bit;
        register long   mask;
        register long   img;
        short           h;
        short           flag;

        for (h = 0; h < height; h++) {
                img = 0L;
                /* The 32-bit row is assembled by four *src++ steps,
                   not from four subscripts. */
                mask = (long) *src++;
                mask |= (long) *src++;
                mask <<= 16;
                mask |= (long) *src++ & 0xffffL;
                mask |= (long) *src++ & 0xffffL;
                flag = 0;
                for (bit = 30; bit > 0; bit--) {
                        if (flag) {
                                img |= bitSet32[bit];
                                if ((mask & bitSet32[bit]) == 0L)
                                        flag = 0;
                        } else if ((mask & bitSet32[bit]) != 0L) {
                                img |= bitSet32[bit + 1];
                                img |= bitSet32[bit];
                                img |= bitSet32[bit - 1];
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
