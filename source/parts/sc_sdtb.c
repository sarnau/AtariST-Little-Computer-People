/*
 * parts/sc_sdtb.c -- shared body; LCP_STX links it in the 0xdece
 * object (in the 0xdece object). Files under parts/ are never compiled
 * standalone.
 */

void
sc_sdtb()
{
        g_srlgb = (void *) Logbase();
        Setscreen(g_srptr, (void *)-1L, -1);
        vswr_mode(vdihnd, MD_REPLACE);
        vsf_interior(vdihnd, FIS_PATTERN);
        vsf_style(vdihnd, FILL_SOLID);
        vsf_color(vdihnd, 0);
}
