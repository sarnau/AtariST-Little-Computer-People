/*
 * parts/p_sftvc.c -- one-line SFX wrapper.  The two revisions order
 * these differently inside their objects, so each configuration
 * includes them in its own order (sound.c for LCP_ORG,
 * asimple.c after a_hello for LCP_STX).
 */

void p_sftvc() { sf_sele(SFX_TV_CLICK,  2L); }
