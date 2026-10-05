/*
 * parts/drawSlot.c -- included by stx_u3.c; never compiled on its own.
 *
 * Only the position is latched in locals; the extents are subscripted
 * at every use.  That is the original's shape -- keep it.
 */
void
drawSlot(index)
short   index;
{
        short   x1;
        short   y1;

        x1 = g_sepex[index];
        y1 = g_sepey[index];

        initMfdb(0L, &g_semfi[index],
                         g_seaim[index], g_seacw[index], g_seach[index]);
        initMfdb(0L, &g_semfm[index],
                         g_seams[index],  g_seacw[index], g_seach[index]);

        blitRect(vdihnd, NOTS_AND_D,
                index * 20 + (long) g_semfm, (long) &g_srmfd,
                0, 0, g_seacw[index] - 1, g_seach[index] - 1,
                x1, y1, x1 + g_seacw[index] - 1, y1 + g_seach[index] - 1);
        blitRect(vdihnd, S_XOR_D,
                index * 20 + (long) g_semfi, (long) &g_srmfd,
                0, 0, g_seacw[index] - 1, g_seach[index] - 1,
                x1, y1, x1 + g_seacw[index] - 1, y1 + g_seach[index] - 1);
}
