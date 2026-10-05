/*
 * Must sit immediately before sfxClick.
 *
 * Included by stx_u2.c; never compiled on its own.
 */
/* Typewriter key click while a letter is being typed. */
void
typeKeySound()
{
        sfxSelect(SFX_TYPEWRITER_KEY, 4L);
}
