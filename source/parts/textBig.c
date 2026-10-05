/* Saves the VDI text attributes to savedTextAttr and sets a 20-pixel
   text height. */
void
textBig()
{
        short   ta, tb, tc, td;
        vqt_attributes(vdiHandle, savedTextAttr);
        /* The four out-pointers are passed in declaration order. */
        vst_height(vdiHandle, 20, &ta, &tb, &tc, &td);
}
