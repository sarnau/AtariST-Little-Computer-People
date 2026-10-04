/*
 * Must sit immediately before sfClick.
 *
 * Included by stx_u2.c; never compiled on its own.
 */
/* Typewriter key click while a letter is being typed. */
void
lt_sets()
{
        sf_sele(SFX_TYPEWRITER_KEY, 4L);
}
