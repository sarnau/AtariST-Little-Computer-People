/*
 * Only the position is latched in locals; the extents are subscripted
 * at every use.  That is the original's shape -- keep it.
 */
void
drawSlot(index)
short   index;
{
        short   x1;
        short   y1;

        x1 = pendX[index];
        y1 = pendY[index];

        initMfdb(0L, &slotImgMfdb[index],
                         drawnImage[index], drawnWidth[index], drawnHeight[index]);
        initMfdb(0L, &slotMaskMfdb[index],
                         drawnMask[index],  drawnWidth[index], drawnHeight[index]);

        blitRect(vdiHandle, NOTS_AND_D,
                index * 20 + (long) slotMaskMfdb, (long) &frameMfdb,
                0, 0, drawnWidth[index] - 1, drawnHeight[index] - 1,
                x1, y1, x1 + drawnWidth[index] - 1, y1 + drawnHeight[index] - 1);
        blitRect(vdiHandle, S_XOR_D,
                index * 20 + (long) slotImgMfdb, (long) &frameMfdb,
                0, 0, drawnWidth[index] - 1, drawnHeight[index] - 1,
                x1, y1, x1 + drawnWidth[index] - 1, y1 + drawnHeight[index] - 1);
}
