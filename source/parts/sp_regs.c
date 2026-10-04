/*
 * Included by stx_u1.c; never compiled on its own.
 */
/* sp_regs: store per-sprite
   pointers and dimensions at slot spriteID, then auto-generate the
   1-bit mask into maskPtr. */
void
sp_regs(spriteID, imgPtr, maskPtr, height, width)
short                   spriteID;
unsigned short *        imgPtr;
unsigned short *        maskPtr;
short                   height;
short                   width;
{
        g_sedim[spriteID] = (short *) imgPtr;
        g_sedms[spriteID]   = (short *) maskPtr;
        g_sedeh[spriteID]             = height;
        g_sedew[spriteID]             = width;
        /* The four values are read back out of the tables instead of
           passing the parameters, as in the original. */
        sp_genma(g_sedim[spriteID], g_sedms[spriteID],
                 g_sedew[spriteID], g_sedeh[spriteID]);
}
