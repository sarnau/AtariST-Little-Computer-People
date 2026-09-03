/*
 * parts/sfClick.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x14786, just before lcp_wkD). Files under parts/ are never
 * compiled standalone.
 */

void
sfClick()
{
        sf_sele(SFX_CLICK, 2L);
}
