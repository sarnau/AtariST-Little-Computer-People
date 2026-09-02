/*
 * parts/fOpen.c -- shared body; LCP_ORG links it in save.c,
 * LCP_STX in the 0xdece object (0x730e).  Files under parts/
 * are never compiled standalone.
 */
/* rwmode: 0=read, 1=write, 2=both.  Three tries with a 1s sleep, then
   Retry alert loop.
   addr: fOpen() */
short
fOpen(filename, rwmode)
char *  filename;
short   rwmode;
{
#ifdef FAITHFUL
        short   fhandle;
        short   retry;
#else
        short   retry;          /* STX declares the counter first */
        short   fhandle;
#endif

        retry = 0;
#ifdef FAITHFUL
        for (;;) {
                fhandle = Fopen(filename, rwmode);
                if (fhandle >= 0)
                        return fhandle;
                retry = retry + 1;
                if (retry < 3)
                        evnt_timer(1000, 0);
                else
                        form_alert(0,
                                "[1][Bad file open.|Try re-booting.][RETRY]");
        }
#else
        /* STX: an explicit backward goto from both arms -- neither
           branch goes through a shared loop-back. */
again:
        fhandle = Fopen(filename, rwmode);
        if (fhandle >= 0)
                return fhandle;
        retry++;
        if (retry < 3) {
                evnt_timer(1000, 0);
                goto again;
        }
        form_alert(0,
                "[1][Bad file open.|Try re-booting.][RETRY]");
        goto again;
#endif
}
