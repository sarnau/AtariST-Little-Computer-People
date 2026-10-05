/* Shows a sprite at once: recomputes the 8-slot layout and copies the
   definition straight into the active slot, bypassing the pending
   double buffer. */

void
activateSprite(spriteId)
short   spriteId;
{
        /* No slot local: the map is subscripted at each use, as in
           the original. */
        layoutSlots();
        drawnImage[spriteSlot[spriteId]]  = spriteBitmap[spriteId];
        drawnMask[spriteSlot[spriteId]]   = spriteMask[spriteId];
        drawnHeight[spriteSlot[spriteId]] = spriteHeight[spriteId];
        drawnWidth[spriteSlot[spriteId]]  = spriteWidth[spriteId];
}
