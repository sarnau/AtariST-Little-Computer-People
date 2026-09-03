/*
 * parts/mq_skip.c -- shared body; LCP_STX links it in at 0x12a, the
 * first function in the MIDI object. Files under parts/ are never
 * compiled standalone.
 */
unsigned char *
mq_skip(ptr)                    /* STX: no second parameter */
unsigned char * ptr;
{
        if (ptr == (unsigned char *) 0)
                return (unsigned char *) 0;

        /* STX steps past the 0x00 first and returns the STEPPED
           pointer when the marker follows. */
        if (*ptr == 0) {
                ptr++;
                if (*ptr == 0xff)
                        return ptr;
        }

        while (*ptr != 0)
                ptr++;
        return ptr;
}
