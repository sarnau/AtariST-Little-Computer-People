/* Build a sprite mask from its colour image.  For each
   16-pixel word group (4 interleaved bitplane words), OR the planes
   -- any non-colour-0 pixel becomes an opaque mask bit -- then
   broadcast the result to all 4 mask planes so the mask has the same
   MFDB stride as the image. */
void
makeMask(imgPtr, maskPtr, width, height)
unsigned short *        imgPtr;
unsigned short *        maskPtr;
unsigned short          width;
unsigned short          height;
{
        /* The words-per-row division is kept in a local of its own and
           every access is *p++, as in the original. */
        unsigned short  wpr;
        unsigned short  n;
        unsigned short  index;
        unsigned short  m;

        wpr = width >> 2;
        n   = (wpr * height) >> 2;
        for (index = 0; index < n; index++) {
                m = *imgPtr++;
                m |= *imgPtr++;
                m |= *imgPtr++;
                m |= *imgPtr++;
                *maskPtr++ = m;
                *maskPtr++ = m;
                *maskPtr++ = m;
                *maskPtr++ = m;
        }
}
