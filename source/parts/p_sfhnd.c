/*
 * parts/p_sfhnd.c -- one-line SFX wrapper.  The two revisions order
 * these differently inside their objects, so each configuration
 * includes them in its own order (sound.c for LCP_ORG,
 * asimple.c after a_hello for LCP_STX).
 */

void p_sfhnd() { sf_sele(SFX_HEAD_NOD,  2L); }
