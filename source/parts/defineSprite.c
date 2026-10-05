/*
 * Included by stx_u1.c; never compiled on its own.
 */
/* defineSprite: store per-sprite
   pointers and dimensions at slot spriteID, then auto-generate the
   1-bit mask into maskPtr. */
void
defineSprite(spriteID, imgPtr, maskPtr, height, width)
short                   spriteID;
unsigned short *        imgPtr;
unsigned short *        maskPtr;
short                   height;
short                   width;
{
        spriteBitmap[spriteID] = (short *) imgPtr;
        spriteMask[spriteID]   = (short *) maskPtr;
        spriteHeight[spriteID]             = height;
        spriteWidth[spriteID]             = width;
        /* The four values are read back out of the tables instead of
           passing the parameters, as in the original. */
        makeMask(spriteBitmap[spriteID], spriteMask[spriteID],
                 spriteWidth[spriteID], spriteHeight[spriteID]);
}
