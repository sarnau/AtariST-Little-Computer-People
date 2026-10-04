/*
 * parts/mq_skip.c -- included by midi_seq.c; never compiled on its own.
 */
unsigned char *
mq_skip(ptr)
unsigned char * ptr;
{
        if (ptr == (unsigned char *) 0)
                return (unsigned char *) 0;

        /* Step past the 0x00 first and return the STEPPED pointer
           when the 0xff marker follows. */
        if (*ptr == 0) {
                ptr++;
                if (*ptr == 0xff)
                        return ptr;
        }

        while (*ptr != 0)
                ptr++;
        return ptr;
}
