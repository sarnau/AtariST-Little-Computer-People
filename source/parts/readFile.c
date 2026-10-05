/*
 * parts/readFile.c -- included by stx_u1.c; never compiled on its own.
 */
/* Fread with retries: read `count' bytes from fhnd into buffer and
   return Fread's result.  A failed read is retried twice after a
   one-second wait; after that a "Bad file read" alert with a single
   RETRY button comes up before every further attempt, so the function
   only returns once a read succeeds. */
short
readFile(fhnd, count, buffer)
short   fhnd;
long    count;
void *  buffer;
{
        short   retry;          /* declared first on purpose: order sets the stack slots */
        short   err;

        retry = 0;
        /* Retry a failed Fread twice after a one-second wait; after
           that, show an alert before every further attempt.  The label + goto shape
           (as in openFile) is the original's and must not become a loop
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
