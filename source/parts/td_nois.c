/*
 * parts/td_nois.c -- shared body; LCP_ORG links it in renderx.c,
 * LCP_STX in the 0xdece object (0x13c74, immediately before td_line).  Files under parts/
 * are never compiled standalone.
 */
/* td_nois: random-colour antenna each frame while TV on.
   Mask (& COLOR_dk_brown = 0xf) clamps to 16-entry palette.
   addr: td_nois() */

void
td_nois()
{
#ifdef FAITHFUL
        long    rnd;

        rnd = Random();
        td_line((short) rnd & COLOR_dk_brown);
#else
        /* STX has no local: the wrapper's result is masked in the
           argument slot. */
        td_line((short) rnd() & COLOR_dk_brown);
#endif
}
