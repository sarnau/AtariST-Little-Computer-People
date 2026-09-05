/*
 * parts/st_titl.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x6d7e, in the 0x400c object after pa_skic). Files under
 * parts/ are never compiled standalone.
 */
/* st_titl: the interactive title screen -- decode TITLE.SCN onto the
   spare screen buffer, then take the owner's name, the date and the
   time from the keyboard.  Nothing is validated until the whole field
   is typed, and a bad field simply re-runs its own entry.
   addr: st_titl() */

void
st_titl()
{
        short   unused;         /* -2, never referenced */
        short   ch;             /* -4  */
        short   fhandle;        /* -6  */
        short   n;              /* -8  */
        short   colour;         /* -10 */
        short   j;              /* -12 */
        short   unused2;        /* -14, never referenced */
        short   unused3;        /* -16, never referenced */

        g_dscp  = sv_phb;
        fhandle = fOpen("title.scn", 0);
        fr_read(fhandle, 2L, &scn_siz);
        scn_buf = (char *) Malloc((long) (scn_siz - 32));
        if (scn_buf == (char *) 0)
                er_nomem();
        fr_read(fhandle, 30L, scn_dic);
        fr_read(fhandle, (long) (scn_siz - 32), scn_buf);
        scn_dec(scn_buf, g_dscp, 16000);
        Mfree(scn_buf);

#ifdef SKIP_TITLE
        /* Test builds only.  The guestbook is interactive -- it waits
           on getKey() for a name, a date, a time and AM/PM -- so an
           unattended Hatari run stalls here for ever.  Seed the fields
           the entry loops would have set and return; TITLE.SCN is
           still decoded above, so the screen buffer and the file path
           are in the same state as a real run.

           A REAL date and time, not the 0-0-0 noon the pre-STX stub
           used: with dt_year 0 the move-in cutscene never finishes and
           the compositor corrupts the screen within a couple of
           minutes.  These are the values a manual run enters --
           09/04/26, 10:30 AM -- and they reach gameplay cleanly.

           NOT part of the shipped configuration: the default build
           must stay byte-identical to DATA/LCP_STX.PRG. */
        lcp.owner_name[0] = 'P';
        lcp.owner_name[1] = 'L';
        lcp.owner_name[2] = 'A';
        lcp.owner_name[3] = 'Y';
        lcp.owner_name[4] = 'E';
        lcp.owner_name[5] = 'R';
        lcp.owner_name[6] = 0;
        dt_mon   = 8;           /* September; st_titl stores month - 1 */
        date_day = 3;           /* the 4th;   likewise day - 1         */
        dt_year  = 26;
        t_hour   = 10;
        t_min    = 30;
        colour = 0; n = 0; j = 0; ch = 0;   /* -Wall: set, never read */
        return;
#else
        colour = 9;
        strPr("NAME: ------------------", 80, 110, colour);
        n = 0;
        while (1) {
                ch = getKey();
                if (ch == 8 && n > 0) {
                        n--;
                        erChr((n << 3) + 128, 110, 15);
                        prCh('-', (n << 3) + 128, 110, colour);
                        continue;
                }
                if (ch == 13 && n > 0)
                        break;
                ch = lcp_upp(ch);
                if (ch < ' ')
                        continue;
                lcp.owner_name[n] = ch;
                erChr((n << 3) + 128, 110, 15);
                prCh(ch, (n << 3) + 128, 110, colour);
                n++;
                if (n == 18)
                        break;
        }
        lcp.owner_name[n] = 0;
        for (j = n; j < 18; j++)
                erChr((j << 3) + 128, 110, 15);

        strPr("ENTER DATE:", 80, 122, colour);
date_entry:
        stEnter(176, 122, "MM/DD/YY", 8, colour);
        dt_mon   = in_str[0] * 10 + in_str[1] - 1;
        date_day = in_str[3] * 10 + in_str[4] - 1;
        dt_year  = in_str[6] * 10 + in_str[7];
        if (dt_mon < 0)
                goto date_entry;
        if (dt_mon >= 12)
                goto date_entry;
        if (date_day < 0)
                goto date_entry;
        if (daysInMo(dt_mon, dt_year) <= date_day)
                goto date_entry;

        strPr("ENTER TIME:", 80, 134, colour);
time_entry:
        stEnter(176, 134, "HH:MM", 5, colour);
        t_hour = in_str[0] * 10 + in_str[1];
        t_min  = in_str[3] * 10 + in_str[4];
        if (t_hour == 0)
                goto time_entry;
        if (t_hour > 12)
                goto time_entry;
        if (t_min > 59)
                goto time_entry;

        strPr("AM OR PM: -M", 80, 146, colour);
        while (1) {
                ch = getKey();
                if (ch == 'A' || ch == 'a') {
                        ch = 'A';
                        if (t_hour == 12)
                                t_hour = 0;
                        break;
                } else if (ch == 'P' || ch == 'p') {
                        ch = 'P';
                        if (t_hour != 12)
                                t_hour += 12;
                        break;
                }
        }
        erChr(160, 146, 15);
        prCh(ch, 160, 146, colour);
        evnt_timer(1000, 0);
#endif
}
