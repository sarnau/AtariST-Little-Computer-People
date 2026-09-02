/*
 * parts/lt_sets.c -- shared body; LCP_ORG links it in sound.c,
 * LCP_STX in the 0xdece object (0x1476c, immediately before sfClick).  Files under parts/
 * are never compiled standalone.
 */
/* addr: lt_sets(), sfClick() */
void
lt_sets()
{
        sf_sele(SFX_TYPEWRITER_KEY, 4L);
}
