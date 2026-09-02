/*
 * parts/sfClick.c -- shared body; LCP_ORG links it in sound.c,
 * LCP_STX in the 0xdece object (0x14786, just before lcp_wkD).  Files under parts/
 * are never compiled standalone.
 */

void
sfClick()
{
        sf_sele(SFX_CLICK, 2L);
}
