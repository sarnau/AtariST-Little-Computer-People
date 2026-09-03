/*
 * parts/psg_cpE.c -- shared body; LCP_STX puts it at 0x1586, at the end
 * of the MIDI object right before psg_upEn, so mq_dise reaches it with
 * a bsr.  Files under parts/ are never compiled standalone.
 */
/* 8-byte memcpy from a .SNG ADSR block into a PSG_ENVELOPE struct.
   addr: psg_cpE() */
void
#ifdef FAITHFUL
psg_cpE(src, dest, count)
unsigned char * src;
unsigned char * dest;
short           count;
{
        while (count != 0) {
                *dest = *src;
                src   = src  + 1;
                dest  = dest + 1;
                count = count - 1;
        }
}
#else
/* STX: a long count, tested by post-decrement, pointers stepped in
   place. */
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
#endif
