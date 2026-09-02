/*
 * parts/sc_sdtf.c -- shared body; LCP_ORG links it in gfx_prim.c,
 * LCP_STX in the 0xdece object (in the 0xdece object).  Files under parts/
 * are never compiled standalone.
 */

void
sc_sdtf()
{
#ifdef FAITHFUL
        Setscreen(g_srlgb, (void *)-1L, -1L);
#else
        Setscreen(g_srlgb, (void *)-1L, -1);
#endif
}
