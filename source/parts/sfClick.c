/*
 * parts/sfClick.c -- included by stx_u2.c; never compiled on its own.
 */

/* Plays the short click effect (SFX_CLICK) -- a key press while the
   resident types on the computer or the typewriter. */
void
sfClick()
{
        sf_sele(SFX_CLICK, 2L);
}
