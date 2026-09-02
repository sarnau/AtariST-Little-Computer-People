/*
 * parts/mq_resp.c -- shared body; LCP_ORG links it in midi_seq.c,
 * LCP_STX in the 0xdece object (0x1184, near the end of the MIDI object).  Files under parts/
 * are never compiled standalone.
 */
/* mq_resp: pre-flight the 16 MIDI channels.  For each physical channel
   0..15, find the first logical channel referencing it, mark its
   program as unset (-1), dispatch a Program Change.
   addr: mq_resp() */

void
mq_resp()
{
#ifdef FAITHFUL
        short   channel;
        short   ch_index;
#else
        /* STX: byte counters, ch_index declared first. */
        char    ch_index;
        char    channel;
#endif

#ifdef FAITHFUL
        for (channel = 0; channel < 16; channel = channel + 1) {
                for (ch_index = 1; ch_index < 16;
                     ch_index = ch_index + 1) {
                        if ((mi_chmap[ch_index] & 0xf) == channel) {
                                g_mcpro[ch_index] = -1;
                                mq_sepc(ch_index);
                                break;
                        }
                }
        }
#else
        /* STX ends the inner scan by forcing the counter, not with
           a break. */
        for (channel = 0; channel < 16; channel++) {
                for (ch_index = 1; ch_index < 16; ch_index++) {
                        if ((mi_chmap[ch_index] & 0xf) == channel) {
                                g_mcpro[ch_index] = -1;
                                mq_sepc(ch_index);
                                ch_index = 15;
                        }
                }
        }
#endif
}
