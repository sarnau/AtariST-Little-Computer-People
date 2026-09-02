/*
 * parts/cntSong.c -- shared body; LCP_ORG links it in init.c,
 * LCP_STX in the 0xdece object (0x400c -- the FIRST function of the 0x400c object).  Files under parts/
 * are never compiled standalone.
 */
/* cntSong: enumerate *.SNG and *.ORG, count into sng_cnt / org_cnt.
   addr: Ghidra count_songs (main step 8). */


void
cntSong()
{
        /* No locals: both results are consumed in place, Fsfirst's
           third argument is a WORD zero, the Fsfirst test is written
           `!Fsfirst(...)` (which tests the word where `== 0` tests the
           long), and the scan is a `while` whose body is just the
           counter step. */
        sng_cnt = 0;
        org_cnt = 0;
        if (!Fsfirst("*.sng", 0)) {
                sng_cnt = 1;
                while (gemdos(0x4F) == 0)
                        sng_cnt++;
        }
        if (!Fsfirst("*.org", 0)) {
                org_cnt = 1;
                while (gemdos(0x4F) == 0)
                        org_cnt++;
        }
}
