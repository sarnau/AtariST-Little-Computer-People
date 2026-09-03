/*
 * parts/p_sfgrt.c -- one-line SFX wrapper.  The two revisions order
 * these differently inside their objects, so each configuration
 * includes them in LCP_STX's order (tvc, spe, hnd, grt).
 */

void p_sfgrt() { sf_sele(SFX_GREETING,  2L); }
