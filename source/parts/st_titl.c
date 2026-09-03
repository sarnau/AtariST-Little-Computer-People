/*
 * parts/st_titl.c -- shared body; LCP_ORG links it in init.c,
 * LCP_STX in the 0xdece object (0x6d7e, in the 0x400c object after pa_skic).  Files under parts/
 * are never compiled standalone.
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
}
