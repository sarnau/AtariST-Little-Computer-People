/*
 * parts/sp_lcpf.c -- shared body; LCP_STX places it in the sprite object
 * (see stx_u3.c for the address).  Files under parts/ are never
 * compiled standalone.
 */
/* sp_lcpf: expand 2-word (32-px) LCP source frame into 4-word (64-px)
   dest row, with optional horizontal mirror.  flipV picks left- vs
   right-half so mirrored frames land at the same screen X.
   Called from sp_updb and sp_lchu.
   addr: sp_lcpf() */

void
sp_lcpf(srcImg, srcMask, destImg, destMask,
                width, height, flipH, flipV)
short * srcImg;
short * srcMask;
short * destImg;
short * destMask;
short   width;
short   height;
short   flipH;
short   flipV;
{
        /* w / m2 / x are register variables (d7/d6/d5); the frame
           holds only mmask, y and m1. */
        register short  w;
        register short  m2;
        register short  x;
        short           mmask;
        short           y;
        short           m1;

        if (flipH == 0) {
                for (y = 0; y < height; y++) {
                        for (x = 0; x < width; x++) {
                                if (flipV != 0) {
                                        *destImg++ = *srcImg++;
                                        *destImg++ = *srcImg++;
                                        *destImg++ = 0;
                                } else {
                                        *destImg++ = 0;
                                        *destImg++ = *srcImg++;
                                        *destImg++ = *srcImg++;
                                }
                                *destImg++ = 0;

                                *destMask++ = *srcMask;
                                *destMask++ = *srcMask;
                                *destMask++ = *srcMask;
                                *destMask++ = *srcMask++;
                        }
                }
                return;
        }

        for (y = 0; y < height; y++) {
                for (x = 0; x < width; x++) {
                        /* The byte halves are selected with explicit
                           `& 0xff` masks, and m2 doubles as the shift
                           temporary for m1. */
                        w  = srcImg[((width - 1) - x) << 1];
                        m2 = rev_tab[w & 0xff];
                        m2 <<= 8;
                        m1 = m2 | rev_tab[(w >> 8) & 0xff];
                        w  = *(srcImg + (((width - 1) - x) << 1) + 1);
                        m2 = rev_tab[w & 0xff];
                        m2 <<= 8;
                        m2 |= rev_tab[(w >> 8) & 0xff];

                        if (flipV != 0) {
                                *destImg++ = m1;
                                *destImg++ = m2;
                                *destImg++ = 0;
                        } else {
                                *destImg++ = 0;
                                *destImg++ = m1;
                                *destImg++ = m2;
                        }
                        *destImg++ = 0;

                        w = srcMask[(width - 1) - x];
                        mmask = rev_tab[w & 0xff] << 8;
                        mmask |= rev_tab[(w >> 8) & 0xff];
                        *destMask++ = mmask;
                        *destMask++ = mmask;
                        *destMask++ = mmask;
                        *destMask++ = mmask;
                }
                /* Plain short* arithmetic: the compiler supplies the
                   ×2 scaling, so the source only shifts once. */
                srcImg  += width << 1;
                srcMask += width;
        }
}
