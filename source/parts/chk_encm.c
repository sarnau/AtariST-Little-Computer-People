/*
 * parts/chk_encm.c -- shared body; LCP_ORG links it in parser.c,
 * LCP_STX in the 0xdece object (0x16f9a, in the 0x148fe object between prCh and prsCmd).  Files under parts/
 * are never compiled standalone.
 */
/* addr: chk_encm() */
short
chk_encm(str)
char *  str;
{
        short   rnd;
        short   entered_word;
        short   action_index;
        short   i;

        /* Clear the accumulated position/bit mask. */
        for (i = 0; i < 10; i = i + 1)
                g_ewb[i] = 0;

        /* Seed the priority from happiness + a small random nudge. */
        rnd = rndRng(0, 3);
        g_aprio = mood_pri[lcp.happiness] + rnd;

        /* Tokenize and mask-accumulate. */
        while ((str = cmd_upp(str, usr_buf)) !=
               (char *) 0) {
                entered_word = chk_vwd(usr_buf);
                if (entered_word == WORD_NONE) {
                        /* Unrecognised word -- +4 priority penalty. */
                        g_aprio = g_aprio + 4;
                } else if (entered_word > 0) {
                        short   pos = ew2pos[entered_word];
                        short   bit = g_ew2b[entered_word];
                        g_ewb[pos] |=
                                bm_lo[bit];
                }
        }

        /* Walk the action-matching table until a row matches or we hit
           the 0xff sentinel. */
        action_index = 0;
        for (;;) {
                if (g_ew2a[action_index].table[0] == 0xff)
                        return ACTION_NONE;
                for (i = 0; i < 10; i = i + 1) {
                        unsigned char   required =
                                g_ew2a[action_index].table[i];
                        if ((g_ewb[i] & required) != required)
                                break;
                }
                if (i >= 10) {
                        g_aprio = g_aprio +
                                g_ew2a[action_index].priority_offset;
                        return (short) (char)
                                g_ew2a[action_index].action;
                }
                action_index = action_index + 1;
        }
}
