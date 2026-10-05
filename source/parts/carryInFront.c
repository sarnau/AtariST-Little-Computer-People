/* Same as carryBehind but in the in-front-of-LCP layer. */

void
carryInFront(g_seix)
short   g_seix;
{
        /* No local for the slot (as in activateSprite): spriteSlot[] is
           re-read at every use. */
        spriteLayer[g_seix] = SPRITE_IN_FRONT;
        layoutSlots();
        drawnImage[spriteSlot[g_seix]] = spriteBitmap[g_seix];
        drawnMask[spriteSlot[g_seix]] = spriteMask[g_seix];
        drawnHeight[spriteSlot[g_seix]] = spriteHeight[g_seix];
        drawnWidth[spriteSlot[g_seix]] = spriteWidth[g_seix];
        isCarrying = YES;
        carriedSprite = g_seix;
}
