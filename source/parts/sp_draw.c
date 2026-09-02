/*
 * parts/sp_draw.c -- LCP_STX links sp_draw in the sprite object at
 * 0x1605c, between putEv and sp_drin; sc_ren8 reaches it with a bsr.
 * Frame -8: only the position is latched, the extents are subscripted
 * at every use.  Files under parts/ are never compiled standalone.
 */
void
sp_draw(index)
short   index;
{
        short   x1;
        short   y1;

        x1 = g_sepex[index];
        y1 = g_sepey[index];

        sp_iniM(0L, &g_semfi[index],
                         g_seaim[index], g_seacw[index], g_seach[index]);
        sp_iniM(0L, &g_semfm[index],
                         g_seams[index],  g_seacw[index], g_seach[index]);

        vroCpyD(vdihnd, NOTS_AND_D,
                index * 20 + (long) g_semfm, (long) &g_srmfd,
                0, 0, g_seacw[index] - 1, g_seach[index] - 1,
                x1, y1, x1 + g_seacw[index] - 1, y1 + g_seach[index] - 1);
        vroCpyD(vdihnd, S_XOR_D,
                index * 20 + (long) g_semfi, (long) &g_srmfd,
                0, 0, g_seacw[index] - 1, g_seach[index] - 1,
                x1, y1, x1 + g_seacw[index] - 1, y1 + g_seach[index] - 1);
}
