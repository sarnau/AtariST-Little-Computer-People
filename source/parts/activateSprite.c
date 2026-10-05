/*
 * parts/activateSprite.c -- included by stx_u2.c; never compiled on its own.
 */

/* Generic sprite activator (save.c, pet animations).
   Recomputes the 8-slot layout and copies the definition into the
   active slot, bypassing the pending double-buffer. */

void
activateSprite(g_seix)
short   g_seix;
{
        /* No slot local: the map is subscripted at each use, as in
           the original. */
        layoutSlots();
        drawnImage[spriteSlot[g_seix]]  = spriteBitmap[g_seix];
        drawnMask[spriteSlot[g_seix]]   = spriteMask[g_seix];
        drawnHeight[spriteSlot[g_seix]] = spriteHeight[g_seix];
        drawnWidth[spriteSlot[g_seix]]  = spriteWidth[g_seix];
}
