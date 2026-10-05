/* resetPrograms: pre-flight the 16 MIDI channels.  For each physical channel
   0..15, find the first logical channel referencing it, mark its
   program as unset (-1), dispatch a Program Change. */

void
resetPrograms()
{
        /* Byte counters, chIndex declared first: the original's
           types and order. */
        char    chIndex;
        char    channel;

        /* The inner scan ends by forcing the counter, not with a
           break, as in the original. */
        for (channel = 0; channel < 16; channel++) {
                for (chIndex = 1; chIndex < 16; chIndex++) {
                        if ((chanMap[chIndex] & 0xf) == channel) {
                                sentProgram[chIndex] = -1;
                                sendProgChange(chIndex);
                                chIndex = 15;
                        }
                }
        }
}
