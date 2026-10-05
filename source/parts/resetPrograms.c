/* resetPrograms: pre-flight the 16 MIDI channels.  For each physical channel
   0..15, find the first logical channel referencing it, mark its
   program as unset (-1), dispatch a Program Change. */

void
resetPrograms()
{
        /* Byte counters, ch_index declared first: the original's
           types and order. */
        char    ch_index;
        char    channel;

        /* The inner scan ends by forcing the counter, not with a
           break, as in the original. */
        for (channel = 0; channel < 16; channel++) {
                for (ch_index = 1; ch_index < 16; ch_index++) {
                        if ((chanMap[ch_index] & 0xf) == channel) {
                                sentProgram[ch_index] = -1;
                                sendProgChange(ch_index);
                                ch_index = 15;
                        }
                }
        }
}
