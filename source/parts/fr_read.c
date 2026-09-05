/*
 * parts/fr_read.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x736c). Files under parts/ are never compiled standalone.
 */
short
fr_read(fhnd, count, buffer)
short   fhnd;
long    count;
void *  buffer;
{
        short   retry;          /* STX declares the counter first */
        short   err;

        retry = 0;
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
}
