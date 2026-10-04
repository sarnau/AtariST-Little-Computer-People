/*
 * parts/fr_read.c -- included by stx_u1.c; never compiled on its own.
 */
short
fr_read(fhnd, count, buffer)
short   fhnd;
long    count;
void *  buffer;
{
        short   retry;          /* declared first on purpose: order sets the stack slots */
        short   err;

        retry = 0;
        /* Retry a failed Fread twice after a one-second wait; after
           that, show an alert before every further attempt.  The label + goto shape
           (as in fOpen) is the original's and must not become a loop
           statement. */
again:
        err = Fread(fhnd, count, buffer);
        if (err >= 0)
                return err;
        retry++;
        if (retry < 3) {
                evnt_timer(1000, 0);
                goto again;
        }
        form_alert(ALERT_NO_DEFAULT,
                "[1][Bad file read.|Try re-booting.][RETRY]");
        goto again;
}
