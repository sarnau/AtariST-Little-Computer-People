/*
 * parts/sp_genma.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x408c, right after cntSong). Files under parts/ are never
 * compiled standalone.
 */
/* sp_genma: Ghidra spritedata_generate_mask_from_color.  For each
   16-pixel word group (4 interleaved bitplane words), OR the planes
   -- any non-colour-0 pixel becomes an opaque mask bit -- then
   broadcast the result to all 4 mask planes so the mask has the same
   MFDB stride as the image. */
void
sp_genma(imgPtr, maskPtr, width, height)
unsigned short *        imgPtr;
unsigned short *        maskPtr;
unsigned short          width;
unsigned short          height;
{
        /* STX's frame is -12: the words-per-row division is kept in a
           local of its own, and every access is *p++. */
        unsigned short  wpr;
        unsigned short  n;
        unsigned short  index;
        unsigned short  m;

        wpr = width >> 2;
        n   = (wpr * height) >> 2;
        for (index = 0; index < n; index++) {
                m  = *imgPtr++;
                m |= *imgPtr++;
                m |= *imgPtr++;
                m |= *imgPtr++;
                *maskPtr++ = m;
                *maskPtr++ = m;
                *maskPtr++ = m;
                *maskPtr++ = m;
        }
}
