/*
 * The first function of the stx_u1 object.
 *
 * Included by stx_u1.c; never compiled on its own.
 */
/* countSongs: enumerate *.SNG and *.ORG, count into songCount / organCount. */


void
countSongs()
{
        /* No locals: both results are consumed in place.  The Fsfirst
           test is written `!Fsfirst(...)` (which tests the word where
           `== 0` tests the long), and the scan is a `while` whose body
           is just the counter step -- all as in the original. */
        songCount = 0;
        organCount = 0;
        if (!Fsfirst("*.sng", F_NORMAL)) {
                songCount = 1;
                while (gemdos(GEMDOS_FSNEXT) == 0)
                        songCount++;
        }
        if (!Fsfirst("*.org", F_NORMAL)) {
                organCount = 1;
                while (gemdos(GEMDOS_FSNEXT) == 0)
                        organCount++;
        }
}
