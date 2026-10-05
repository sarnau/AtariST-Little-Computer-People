/* parseSongHeader: walk header from songEvents to first 0xFF.
   Commands: 0x80/0x81/0x83/0x84 (config), 0xC0 (program change),
   0x01..0x7F (note-stride skip, 3 bytes).  Also parses the 90-byte
   channel/program-map block preceding the header events. */

void
parseSongHeader(p)
unsigned char * p;
{
        unpackChanMap(p - 90);

        /* Skip a leading zero byte (used in .sng files where the
           channel-map block is padded to an even boundary). */
        if (*p == 0)
                p++;

        while (*p != 0) {
                /* Bytes in the note-event range 0x01..0x7F -- and
                   0xA0..0xFE via the & 0x9f mask that the 1985 code
                   used -- are 3-byte note events.  Skip past them. */
                if ((*p & 0x9f) < 0x20) {
                        p += 3;
                        continue;
                }

                /* Config-command dispatch.  The selector is masked to
                   a byte, the handlers are written INLINE as the case
                   bodies, and there is no default arm -- an unknown byte
                   falls straight to the loop test.  All three are the
                   original's shape. */
                switch (*p & 0xff) {
                case MIDI_HDR_SET_KEY:
                        buildNoteMap(songKey = p[2]);
                        p += 3;
                        break;
                case MIDI_HDR_SET_TEMPO:
                        songTempo = p[1] & 0xff;
                        ticksPerBeat = 2400;
                        ticksPerBeat /= songTempo;
                        p += 2;
                        break;
                case MIDI_HDR_SET_VOLUME:
                        p += 2;
                        break;
                case MIDI_HDR_SET_VELOCITY:
                        defVelocity = p[2];
                        if      (defVelocity < 0x17) defPsgVol = 5;
                        else if (defVelocity < 0x27) defPsgVol = 7;
                        else if (defVelocity < 0x37) defPsgVol = 9;
                        else if (defVelocity < 0x57) defPsgVol = 11;
                        else if (defVelocity < 0x67) defPsgVol = 13;
                        /* Alcyon narrows 0x80 to a signed byte, so
                           this compare is trivially true and the
                           store is dead -- kept, the original has it. */
                        else if (defVelocity < 0x80) defPsgVol = 15;
                        p += 3;
                        break;
                case MIDI_HDR_PROGRAM_CHANGE:
                        p += 3;
                        break;
                case MIDI_HDR_END:
                        return;
                        /* Unreachable, but it must stay: the original
                           has this dead branch to the switch end.
                           Together with the missing default arm it
                           reproduces both the branch and the jump
                           table's default target. */
                        break;
                }
        }
}
