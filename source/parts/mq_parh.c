/*
 * parts/mq_parh.c -- included by midi_seq.c; never compiled on its own.
 */
/* mq_parh: walk header from mi_dbase to first 0xFF.
   Commands: 0x80/0x81/0x83/0x84 (config), 0xC0 (program change),
   0x01..0x7F (note-stride skip, 3 bytes).  Also parses the 90-byte
   channel/program-map block preceding the header events. */

void
mq_parh(p)
unsigned char * p;
{
        mq_pacm(p - 90);

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
                        mq_bust(g_mkey = p[2]);
                        p += 3;
                        break;
                case MIDI_HDR_SET_TEMPO:
                        mi_temp = p[1] & 0xff;
                        g_mtspb = 2400;
                        g_mtspb /= mi_temp;
                        p += 2;
                        break;
                case MIDI_HDR_SET_VOLUME:
                        p += 2;
                        break;
                case MIDI_HDR_SET_VELOCITY:
                        mi_dvel = p[2];
                        if      (mi_dvel < 0x17) psg_dvol = 5;
                        else if (mi_dvel < 0x27) psg_dvol = 7;
                        else if (mi_dvel < 0x37) psg_dvol = 9;
                        else if (mi_dvel < 0x57) psg_dvol = 11;
                        else if (mi_dvel < 0x67) psg_dvol = 13;
                        /* Alcyon narrows 0x80 to a signed byte, so
                           this compare is trivially true and the
                           store is dead -- kept, the original has it. */
                        else if (mi_dvel < 0x80) psg_dvol = 15;
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
