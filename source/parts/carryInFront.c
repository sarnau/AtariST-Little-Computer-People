/* Same as carryBehind but in the in-front-of-LCP layer. */

void
carryInFront(spriteId)
short   spriteId;
{
        /* No local for the slot (as in activateSprite): spriteSlot[] is
           re-read at every use. */
        spriteLayer[spriteId] = SPRITE_IN_FRONT;
        layoutSlots();
        drawnImage[spriteSlot[spriteId]] = spriteBitmap[spriteId];
        drawnMask[spriteSlot[spriteId]] = spriteMask[spriteId];
        drawnHeight[spriteSlot[spriteId]] = spriteHeight[spriteId];
        drawnWidth[spriteSlot[spriteId]] = spriteWidth[spriteId];
        isCarrying = YES;
        carriedSprite = spriteId;
}
