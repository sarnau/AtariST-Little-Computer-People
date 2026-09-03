/*
 * parts/p_sfhnd.c -- one-line SFX wrapper.  The two revisions order
 * these differently inside their objects, so each configuration
 * includes them in LCP_STX's order (tvc, spe, hnd, grt).
 */

void p_sfhnd() { sf_sele(SFX_HEAD_NOD,  2L); }
