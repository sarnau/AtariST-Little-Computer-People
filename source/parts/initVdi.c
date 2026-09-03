/*
 * parts/initVdi.c -- shared body; LCP_STX 0x764e, right after
 * rst_vsth.  Files under parts/ are never compiled standalone.
 */
void
initVdi()
{
        sv_lgb = (void *) Logbase();
        Setscreen(g_dscp, (void *)-1L, -1);     /* rez as word */
        vswr_mode(vdihnd, 1);
        vsf_interior(vdihnd, 2);        /* STX: FILL_PATTERN */
        vsf_style(vdihnd, 8);           /* STX: style 8 */
        vsf_color(vdihnd, vdi_colt[0xc]);
}
