/*
 * parts/mq_parh.c -- shared body; LCP_ORG links it in midi_seq.c,
 * LCP_STX at 0x11fa, near the end of the MIDI object.  Files under parts/ are never compiled
 * standalone.
 */
/* mq_parh: walk header from mi_dbase to first 0xFF.
   Commands: 0x80/0x81/0x83/0x84 (config), 0xC0 (program change),
   0x01..0x7F (note-stride skip, 3 bytes).  Also parses the 90-byte
   channel/program-map block preceding the header events.
   addr: mq_parh() */

void
mq_parh(p)
unsigned char * p;
{
        mq_pacm(p - 90);

        /* Skip a leading zero byte (used in .sng files where the
           channel-map block is padded to an even boundary). */
        /* STX: p++ straight to the frame slot, and the scan is a
           `while (*p)` with the test at the bottom. */
#ifdef FAITHFUL
        if (*p == 0)
                p = p + 1;

        for (;;) {
                if (*p == 0)
                        return;

#else
        if (*p == 0)
                p++;

        while (*p != 0) {
#endif
                /* Bytes in the note-event range 0x01..0x7F -- and
                   0xA0..0xFE via the & 0x9f mask that the 1985 code
                   used -- are 3-byte note events.  Skip past them. */
                /* STX tests only the upper bound (the while already
                   excludes 0) and advances with p += 3. */
#ifdef FAITHFUL
                if ((*p & 0x9f) < 0x20 && (*p & 0x9f) != 0) {
                        p = p + 3;
                        continue;
                }
#else
                if ((*p & 0x9f) < 0x20) {
                        p += 3;
                        continue;
                }
#endif

                /* Config-command dispatch.  STX masks the selector
                   to a byte (and.w #255 before the compare chain). */
#ifdef FAITHFUL
                switch (*p) {
                case MIDI_HDR_SET_CHANNEL_COUNT:
                        p = mh_chac(p);
                        break;
                case MIDI_HDR_SET_TEMPO:
                        p = mh_temp(p);
                        break;
                case MIDI_HDR_SET_VOLUME:
                        p = mh_volu(p);
                        break;
                case MIDI_HDR_BUILD_SCALE_TABLE:
                        p = mh_scat(p);
                        break;
                case MIDI_HDR_PROGRAM_CHANGE:
                        p = mh_proc(p);
                        break;
                case MIDI_HDR_END:
                        return;
                default:
                        p = p + 1;
                        break;
                }
#else
                /* STX writes the handlers INLINE as the case bodies
                   (the jump table targets 0x1246/0x1264/0x129c/
                   0x12a4/0x132c are inside mq_parh), and has no
                   default arm -- an unknown byte falls straight to
                   the loop test.  The port factored the same code
                   into the static mh_* helpers, which is why they
                   carry other-image addresses in their comments. */
                switch (*p & 0xff) {
                case MIDI_HDR_SET_CHANNEL_COUNT:        /* 0x1246 */
                        mq_bust(g_mchcn = p[2]);
                        p += 3;
                        break;
                case MIDI_HDR_SET_TEMPO:                /* 0x1264 */
                        mi_temp = p[1] & 0xff;
                        g_mtspb = 2400;
                        g_mtspb /= mi_temp;
                        p += 2;
                        break;
                case MIDI_HDR_SET_VOLUME:               /* 0x129c */
                        p += 2;
                        break;
                case MIDI_HDR_BUILD_SCALE_TABLE:        /* 0x12a4 */
                        mi_dvel = p[2];
                        if      (mi_dvel < 0x17) psg_dvol = 5;
                        else if (mi_dvel < 0x27) psg_dvol = 7;
                        else if (mi_dvel < 0x37) psg_dvol = 9;
                        else if (mi_dvel < 0x57) psg_dvol = 11;
                        else if (mi_dvel < 0x67) psg_dvol = 13;
                        /* Alcyon narrows 0x80 to a signed byte, so
                           this compare is trivially true and the
                           store is dead -- STX emits it all the
                           same. */
                        else if (mi_dvel < 0x80) psg_dvol = 15;
                        p += 3;
                        break;
                case MIDI_HDR_PROGRAM_CHANGE:           /* 0x132c */
                        p += 3;
                        break;
                case MIDI_HDR_END:                      /* 0x1332 */
                        return;
                default:
                        break;
                }
#endif
        }
}
