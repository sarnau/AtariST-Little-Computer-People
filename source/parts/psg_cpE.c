/*
 * Must sit right before psg_upEn, near mq_dise, which calls it.
 *
 * Included by midi_seq.c; never compiled on its own.
 */
/* 8-byte memcpy from a .SNG ADSR block into a PSG_ENVELOPE struct. */
void
/* A long count, tested by post-decrement, pointers stepped in place:
   all as in the original. */
psg_cpE(src, dest, count)
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
