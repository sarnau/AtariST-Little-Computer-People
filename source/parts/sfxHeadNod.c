/*
 * parts/sfxHeadNod.c -- one-line SFX wrapper.  The four wrappers are
 * included by stx_u2.c in the original's order (tvc, spe, hnd, grt),
 * which must not change.
 */

void sfxHeadNod() { sfxSelect(SFX_HEAD_NOD,  2L); }
