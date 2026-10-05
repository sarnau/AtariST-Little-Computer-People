/* Activate sprite as carried object in behind-LCP layer.
   The per-frame X/Y update happens in gameTick's carrying path. */

void
carryBehind(spriteId)
short   spriteId;
{
        spriteLayer[spriteId] = SPRITE_BEHIND_LCP;
        layoutSlots();
        drawnImage[spriteSlot[spriteId]] = spriteBitmap[spriteId];
        drawnMask[spriteSlot[spriteId]] = spriteMask[spriteId];
        drawnHeight[spriteSlot[spriteId]] = spriteHeight[spriteId];
        drawnWidth[spriteSlot[spriteId]] = spriteWidth[spriteId];
        isCarrying = YES;
        carriedSprite = spriteId;
}
