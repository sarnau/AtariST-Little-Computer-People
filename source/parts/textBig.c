/*
 * parts/textBig.c -- included by games.c; never compiled on its own.
 * Saves the VDI text attributes to sv_vqta and sets a 20-pixel text
 * height.
 */
void
textBig()
{
        short   ta, tb, tc, td;
        vqt_attributes(vdihnd, sv_vqta);
        /* The four out-pointers are passed in declaration order. */
        vst_height(vdihnd, 20, &ta, &tb, &tc, &td);
}
