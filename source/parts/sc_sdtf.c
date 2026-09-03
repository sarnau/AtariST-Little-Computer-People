/*
 * parts/sc_sdtf.c -- shared body; LCP_STX links it in the 0xdece
 * object (in the 0xdece object). Files under parts/ are never compiled
 * standalone.
 */

void
sc_sdtf()
{
        Setscreen(g_srlgb, (void *)-1L, -1);
}
