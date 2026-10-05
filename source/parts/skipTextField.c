/*
 * parts/skipTextField.c -- included by midi_seq.c; never compiled on its own.
 */
/* Steps over a zero-terminated text field in song data and returns a
   pointer to its terminating 0x00, so startSong can hand the start of
   the event stream to initSongState.  An empty field (a 0x00 followed by the
   0xff marker) returns a pointer to the 0xff instead.  NULL in gives
   NULL out. */
unsigned char *
skipTextField(ptr)
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
