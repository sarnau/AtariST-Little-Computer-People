/* carryBehind: activate sprite as carried object in behind-LCP layer.
   The per-frame X/Y update happens in gameTick's carrying path. */

void
carryBehind(g_seix)
short   g_seix;
{
        spriteLayer[g_seix] = SPRITE_BEHIND_LCP;
        layoutSlots();
        drawnImage[spriteSlot[g_seix]] = spriteBitmap[g_seix];
        drawnMask[spriteSlot[g_seix]] = spriteMask[g_seix];
        drawnHeight[spriteSlot[g_seix]] = spriteHeight[g_seix];
        drawnWidth[spriteSlot[g_seix]] = spriteWidth[g_seix];
        isCarrying = YES;
        carriedSprite = g_seix;
}
