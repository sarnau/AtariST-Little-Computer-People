/*
 * parts/st_titl.c -- shared body; LCP_ORG links it in init.c,
 * LCP_STX in the 0xdece object (0x6d7e, in the 0x400c object after pa_skic).  Files under parts/
 * are never compiled standalone.
 */
/* st_titl (ROM 0x7fae): in THIS binary the "title screen" is a stub
   that defaults the owner name to "PLAYER" and the clock to noon,
   0-0-0 -- there is no interactive name/date/time entry.  (The
   916-byte interactive version previously here came from the other
   Ghidra image; its TOS v_gtext crash makes sense in hindsight.)
   addr: st_titl() */

void
st_titl()
{
        short   i;

        lcp.owner_name[0] = 'P';
        lcp.owner_name[1] = 'L';
        lcp.owner_name[2] = 'A';
        lcp.owner_name[3] = 'Y';
        lcp.owner_name[4] = 'E';
        lcp.owner_name[5] = 'R';
        for (i = 6; i < 24; i = i + 1)
                lcp.owner_name[i] = 0;
        dt_mon   = 0;
        date_day = 0;
        dt_year  = 0;
        t_hour   = 12;
        t_min    = 0;
}
