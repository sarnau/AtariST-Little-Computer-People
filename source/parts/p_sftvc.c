/*
 * parts/p_sftvc.c -- one-line SFX wrapper.  The two revisions order
 * these differently inside their objects, so each configuration
 * includes them in LCP_STX's order (tvc, spe, hnd, grt).
 */

void p_sftvc() { sf_sele(SFX_TV_CLICK,  2L); }
