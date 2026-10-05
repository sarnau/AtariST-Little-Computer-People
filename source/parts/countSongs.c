/*
 * The first function of the stx_u1 object.
 *
 * Included by stx_u1.c; never compiled on its own.
 */
/* countSongs: enumerate *.SNG and *.ORG, count into sng_cnt / org_cnt. */


void
countSongs()
{
        /* No locals: both results are consumed in place.  The Fsfirst
           test is written `!Fsfirst(...)` (which tests the word where
           `== 0` tests the long), and the scan is a `while` whose body
           is just the counter step -- all as in the original. */
        sng_cnt = 0;
        org_cnt = 0;
        if (!Fsfirst("*.sng", F_NORMAL)) {
                sng_cnt = 1;
                while (gemdos(GEMDOS_FSNEXT) == 0)
                        sng_cnt++;
        }
        if (!Fsfirst("*.org", F_NORMAL)) {
                org_cnt = 1;
                while (gemdos(GEMDOS_FSNEXT) == 0)
                        org_cnt++;
        }
}
