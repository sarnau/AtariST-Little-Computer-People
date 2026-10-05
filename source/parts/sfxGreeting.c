/*
 * One-line SFX wrapper.  The four wrappers must stay in the order
 * tvc, spe, hnd, grt.
 * Included by stx_u2.c; never compiled on its own.
 */

void sfxGreeting() { sfxSelect(SFX_GREETING,  2L); }
