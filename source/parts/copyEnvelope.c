/*
 * Must sit right before stepEnvelopes, near sendMidiEvent, which calls it.
 */
/* 8-byte memcpy from a .SNG ADSR block into a PSG_ENVELOPE struct. */
void
/* A long count, tested by post-decrement, pointers stepped in place:
   all as in the original. */
copyEnvelope(src, dest, count)
unsigned char * src;
unsigned char * dest;
long            count;
{
        while (count--) {
                *dest = *src;
                src++;
                dest++;
        }
}
