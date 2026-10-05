/* titleScreen: the interactive title screen -- decode TITLE.SCN onto the
   spare screen buffer, then take the owner's name, the date and the
   time from the keyboard.  Nothing is validated until the whole field
   is typed, and a bad field simply re-runs its own entry. */

void
titleScreen()
{
        /* Declaration order, and the three unused locals, must stay:
           they fix the stack frame the original has. */
        short   unused;         /* never referenced */
        short   ch;
        short   fhandle;
        short   n;
        short   colour;
        short   j;
        short   unused2;        /* never referenced */
        short   unused3;        /* never referenced */

        stripBuf = tosPhysbase;
        fhandle = openFile("title.scn", RMODE_RD);
        readFile(fhandle, 2L, &scnSize);
        scnBuffer = (char *) Malloc((long) (scnSize - 32));
        if (scnBuffer == (char *) 0)
                outOfMemory();
        readFile(fhandle, 30L, scnDict);
        readFile(fhandle, (long) (scnSize - 32), scnBuffer);
        decodeScn(scnBuffer, stripBuf, 16000);
        Mfree(scnBuffer);

#ifdef SKIP_TITLE
        /* Test builds only.  The guestbook is interactive -- it waits
           on getKey() for a name, a date, a time and AM/PM -- so an
           unattended Hatari run stalls here for ever.  Seed the fields
           the entry loops would have set and return; TITLE.SCN is
           still decoded above, so the screen buffer and the file path
           are in the same state as a real run.

           A REAL date and time: with t_year 0 the move-in cutscene
           never finishes and the compositor corrupts the screen within
           a couple of minutes.  These are the values a manual run
           enters -- 09/04/26, 10:30 AM -- and they reach gameplay
           cleanly.

           NOT part of the shipped configuration: the default build
           must stay byte-identical to the original. */
        resident.owner_name[0] = 'P';
        resident.owner_name[1] = 'L';
        resident.owner_name[2] = 'A';
        resident.owner_name[3] = 'Y';
        resident.owner_name[4] = 'E';
        resident.owner_name[5] = 'R';
        resident.owner_name[6] = 0;
        t_mon = 8;           /* September; titleScreen stores month - 1 */
        t_day = 3;           /* the 4th;   likewise day - 1         */
        t_year = 26;
        t_hour = 10;
        t_min = 30;
        colour = 0; n = 0; j = 0; ch = 0;   /* -Wall: set, never read */
        return;
#else
        colour = 9;
        printString("NAME: ------------------", 80, 110, colour);
        n = 0;
        while (1) {
                ch = getKey();
                if (ch == 8 && n > 0) {
                        n--;
                        eraseChar((n << 3) + 128, 110, 15);
                        printChar('-', (n << 3) + 128, 110, colour);
                        continue;
                }
                if (ch == 13 && n > 0)
                        break;
                ch = toUpper(ch);
                if (ch < ' ')
                        continue;
                resident.owner_name[n] = ch;
                eraseChar((n << 3) + 128, 110, 15);
                printChar(ch, (n << 3) + 128, 110, colour);
                n++;
                if (n == 18)
                        break;
        }
        resident.owner_name[n] = 0;
        for (j = n; j < 18; j++)
                eraseChar((j << 3) + 128, 110, 15);

        printString("ENTER DATE:", 80, 122, colour);
date_entry:
        enterField(176, 122, "MM/DD/YY", 8, colour);
        t_mon = inputLine[0] * 10 + inputLine[1] - 1;
        t_day = inputLine[3] * 10 + inputLine[4] - 1;
        t_year = inputLine[6] * 10 + inputLine[7];
        if (t_mon < 0)
                goto date_entry;
        if (t_mon >= 12)
                goto date_entry;
        if (t_day < 0)
                goto date_entry;
        if (daysInMonth(t_mon, t_year) <= t_day)
                goto date_entry;

        printString("ENTER TIME:", 80, 134, colour);
time_entry:
        enterField(176, 134, "HH:MM", 5, colour);
        t_hour = inputLine[0] * 10 + inputLine[1];
        t_min  = inputLine[3] * 10 + inputLine[4];
        if (t_hour == 0)
                goto time_entry;
        if (t_hour > 12)
                goto time_entry;
        if (t_min > 59)
                goto time_entry;

        printString("AM OR PM: -M", 80, 146, colour);
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
        eraseChar(160, 146, 15);
        printChar(ch, 160, 146, colour);
        evnt_timer(1000, 0);
#endif
}
