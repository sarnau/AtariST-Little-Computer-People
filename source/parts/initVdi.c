/*
 * parts/initVdi.c -- included by games.c, right after rst_vsth; never
 * compiled on its own.
 */
void
initVdi()
{
        sv_lgb = (void *) Logbase();
        Setscreen(g_dscp, (void *)-1L, -1);     /* rez as word */
        vswr_mode(vdihnd, MD_REPLACE);
        vsf_interior(vdihnd, FIS_PATTERN);
        vsf_style(vdihnd, FILL_SOLID);
        vsf_color(vdihnd, vdi_colt[0xc]);
}
