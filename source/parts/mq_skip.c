/*
 * parts/mq_skip.c -- shared body; LCP_ORG links it in midi_seq.c,
 * LCP_STX at 0x12a, the first function in the MIDI object.  Files under parts/ are never compiled
 * standalone.
 */
#ifdef FAITHFUL
unsigned char *
mq_skip(ptr, position)
unsigned char * ptr;
long            position;
{
        (void) position;
#else
unsigned char *
mq_skip(ptr)                    /* STX: no second parameter */
unsigned char * ptr;
{
#endif
        if (ptr == (unsigned char *) 0)
                return (unsigned char *) 0;

#ifdef FAITHFUL
        if (ptr[0] == 0 && ptr[1] == 0xff)
                return ptr;

        while (*ptr != 0)
                ptr = ptr + 1;
#else
        /* STX steps past the 0x00 first and returns the STEPPED
           pointer when the marker follows. */
        if (*ptr == 0) {
                ptr++;
                if (*ptr == 0xff)
                        return ptr;
        }

        while (*ptr != 0)
                ptr++;
#endif
        return ptr;
}
