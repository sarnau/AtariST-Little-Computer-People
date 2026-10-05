/* Random-colour antenna each frame while TV on.
   Mask (& COLOR_dk_brown = 0xf) clamps to 16-entry palette. */

void
tvNoise()
{
        /* No local: the wrapper's result is masked inside the
           argument expression. */
        drawTvPicture((short) rnd() & COLOR_dk_brown);
}
