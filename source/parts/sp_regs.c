/*
 * parts/sp_regs.c -- shared body; LCP_ORG links it in sprload.c,
 * LCP_STX in the 0xdece object (0x5bdc, in the 0x400c object between lc_load and gameLoop).  Files under parts/
 * are never compiled standalone.
 */
/* sp_regs: Ghidra spritedata_create_with_mask.  Store per-sprite
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
        /* STX reads the four values back out of the tables instead of
           passing the parameters. */
        sp_genma(g_sedim[spriteID], g_sedms[spriteID],
                 g_sedew[spriteID], g_sedeh[spriteID]);
}
