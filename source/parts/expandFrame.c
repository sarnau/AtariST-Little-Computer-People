/* Expands a resident frame from the two bitplanes it is stored with
   to the four the compositor draws.  width counts 16-pixel chunks; each
   source chunk is two plane words, which go to planes 0-1 when
   lowPlanes is set (the body: colours 1-3, so palette slots 1 and 2 give
   the clothes) or to planes 1-2 otherwise (the head: colours 2, 4 and 6,
   slot 6 being the skin).  The unused planes are cleared, and the
   chunk's mask word is copied into all four.  With mirror set the
   chunks are taken right to left and every word bit-reversed through
   mirrorTable.  Called from updateBody and updateHead. */

void
expandFrame(srcImg, srcMask, destImg, destMask,
                width, height, mirror, lowPlanes)
short * srcImg;
short * srcMask;
short * destImg;
short * destMask;
short   width;
short   height;
short   mirror;
short   lowPlanes;
{
        /* w, planeB and x must stay register variables, declared in this
           order; only mirMask, y and planeA live on the stack. */
        register short  w;
        register short  planeB;
        register short  x;
        short           mirMask;
        short           y;
        short           planeA;

        if (mirror == 0) {
                for (y = 0; y < height; y++) {
                        for (x = 0; x < width; x++) {
                                if (lowPlanes != 0) {
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
                           `& 0xff` masks, and planeB doubles as the shift
                           temporary for planeA. */
                        w  = srcImg[((width - 1) - x) << 1];
                        planeB = mirrorTable[w & 0xff];
                        planeB <<= 8;
                        planeA = planeB | mirrorTable[(w >> 8) & 0xff];
                        w  = *(srcImg + (((width - 1) - x) << 1) + 1);
                        planeB = mirrorTable[w & 0xff];
                        planeB <<= 8;
                        planeB |= mirrorTable[(w >> 8) & 0xff];

                        if (lowPlanes != 0) {
                                *destImg++ = planeA;
                                *destImg++ = planeB;
                                *destImg++ = 0;
                        } else {
                                *destImg++ = 0;
                                *destImg++ = planeA;
                                *destImg++ = planeB;
                        }
                        *destImg++ = 0;

                        w = srcMask[(width - 1) - x];
                        mirMask = mirrorTable[w & 0xff] << 8;
                        mirMask |= mirrorTable[(w >> 8) & 0xff];
                        *destMask++ = mirMask;
                        *destMask++ = mirMask;
                        *destMask++ = mirMask;
                        *destMask++ = mirMask;
                }
                /* Plain short* arithmetic: the compiler supplies the
                   x2 scaling, so the source only shifts once. */
                srcImg  += width << 1;
                srcMask += width;
        }
}
