/*
 * parts/sc_sdtb.c -- shared body; LCP_ORG links it in gfx_prim.c,
 * LCP_STX in the 0xdece object (in the 0xdece object).  Files under parts/
 * are never compiled standalone.
 */

void
sc_sdtb()
{
        g_srlgb = (void *) Logbase();
#ifdef FAITHFUL
        Setscreen(g_srptr, (void *)-1L, -1L);
#else
        Setscreen(g_srptr, (void *)-1L, -1);
#endif
        vswr_mode(vdihnd, 1);
#ifdef FAITHFUL
        vsf_interior(vdihnd, 1);        /* ROM: FIS_SOLID, not PATTERN */
        vsf_style(vdihnd, 1);           /* ROM: 1, not 8 */
#else
        vsf_interior(vdihnd, 2);        /* STX: FIS_PATTERN */
        vsf_style(vdihnd, 8);
#endif
        vsf_color(vdihnd, 0);
}
