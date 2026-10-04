/*
 * parts/rst_vsth.c -- included by games.c; never compiled on its own.
 * Restores the VDI text height from sv_vqta[7] (the cell height).
 */
void
rst_vsth()
{
        short   ta, tb, tc, td;
        vst_height(vdihnd, sv_vqta[7], &ta, &tb, &tc, &td);
}
