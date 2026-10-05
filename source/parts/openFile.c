/*
 * Included by stx_u1.c; never compiled on its own.
 */
/* rwmode: 0=read, 1=write, 2=both.  Three tries with a 1s sleep, then
   Retry alert loop. */
short
openFile(filename, rwmode)
char *  filename;
short   rwmode;
{
        short   retry;          /* declared first, as in the original */
        short   fhandle;

        retry = 0;
        /* An explicit backward goto from both arms rather than a loop:
           neither branch goes through a shared loop-back, as in the
           original. */
again:
        fhandle = Fopen(filename, rwmode);
        if (fhandle >= 0)
                return fhandle;
        retry++;
        if (retry < 3) {
                evnt_timer(1000, 0);
                goto again;
        }
        form_alert(ALERT_NO_DEFAULT,
                "[1][Bad file open.|Try re-booting.][RETRY]");
        goto again;
}
