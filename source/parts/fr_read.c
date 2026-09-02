/*
 * parts/fr_read.c -- shared body; LCP_ORG links it in save.c,
 * LCP_STX in the 0xdece object (0x736c).  Files under parts/
 * are never compiled standalone.
 */
fr_read(fhnd, count, buffer)
short   fhnd;
long    count;
void *  buffer;
{
#ifdef FAITHFUL
        short   err;
        short   retry;
#else
        short   retry;          /* STX declares the counter first */
        short   err;
#endif

        retry = 0;
#ifdef FAITHFUL
        for (;;) {
                /* Fread expects handle as word; a (long) cast here pushes
                   4 bytes where TOS wants 2 and silently reads from
                   handle 0.  Keep fhnd as short. */
                err = Fread(fhnd, count, buffer);
                if (err >= 0)
                        return;
                retry = retry + 1;
                if (retry < 3)
                        evnt_timer(1000, 0);
                else
                        form_alert(0,
                                "[1][Bad file read.|Try re-booting.][RETRY]");
        }
#else
        /* STX: same explicit-goto retry loop as fOpen, returning the
           Fread result. */
again:
        err = Fread(fhnd, count, buffer);
        if (err >= 0)
                return err;
        retry++;
        if (retry < 3) {
                evnt_timer(1000, 0);
                goto again;
        }
        form_alert(0,
                "[1][Bad file read.|Try re-booting.][RETRY]");
        goto again;
#endif
}
