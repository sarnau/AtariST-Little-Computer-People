/* 8-byte memcpy from a .SNG ADSR block into a PSG_ENVELOPE struct.  A
   long count, tested by post-decrement, pointers stepped in place: all
   as in the original.

   Must sit right before stepEnvelopes, near sendMidiEvent, which calls
   it. */
void
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
