/*
 * Restores the VDI text height from savedTextAttr[7] (the cell
 * height).
 */
void
textNormal()
{
        short   ta, tb, tc, td;
        vst_height(vdiHandle, savedTextAttr[7], &ta, &tb, &tc, &td);
}
