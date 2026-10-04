/*
 * parts/initVdi.c -- shared body; LCP_STX 0x764e, right after
 * rst_vsth.  Files under parts/ are never compiled standalone.
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
