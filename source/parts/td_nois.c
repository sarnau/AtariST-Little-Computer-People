/*
 * parts/td_nois.c -- included by stx_u2.c; never compiled on its own.
 */
/* td_nois: random-colour antenna each frame while TV on.
   Mask (& COLOR_dk_brown = 0xf) clamps to 16-entry palette. */

void
td_nois()
{
        /* No local: the wrapper's result is masked inside the
           argument expression. */
        td_line((short) rnd() & COLOR_dk_brown);
}
