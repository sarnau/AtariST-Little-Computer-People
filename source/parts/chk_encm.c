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
        /* STX's frame is -10: no `rnd` temporary, and the priority
           seed adds the roll FIRST. */
        short   i;
        short   action_index;
        short   entered_word;

        /* Clear the accumulated position/bit mask. */
        for (i = 0; i < 10; i++)
                g_ewb[i] = 0;

        /* Seed the priority from happiness + a small random nudge. */
        g_aprio = rndRng(0, 3) + mood_pri[lcp.happiness];

        /* Tokenize and mask-accumulate.  STX breaks out of a
           `while (1)`; the store's own flags drive the test. */
        while (1) {
                if ((str = cmd_upp(str, usr_buf)) == (char *) 0)
                        break;
                /* STX tests the store's own flags, so the
                   "unrecognised" sentinel here is 0 -- even though
                   chk_vwd returns -1 when it runs off the table. */
                if ((entered_word = chk_vwd(usr_buf)) == 0) {
                        /* Unrecognised word -- +4 priority penalty. */
                        g_aprio += 4;
                } else if (entered_word > 0) {
                        /* Both index tables are char[] in STX, and
                           there are no temporaries. */
                        g_ewb[ew2pos[entered_word]] |=
                                bm_lo[g_ew2b[entered_word]];
                }
        }

        /* Walk the action-matching table until a row matches or we hit
           the 0xff sentinel.  The walk is a `while (1)` whose sentinel
           test breaks to the ACTION_NONE return placed after the loop;
           a row that fails jumps straight to the increment through an
           explicit goto, not a break plus an `i >= 10` re-test. */
        action_index = 0;
        while (1) {
                if (g_ew2a[action_index].table[0] == 0xff)
                        break;
                for (i = 0; i < 10; i++)
                        if ((g_ew2a[action_index].table[i] & g_ewb[i]) !=
                            g_ew2a[action_index].table[i])
                                goto next;
                g_aprio += g_ew2a[action_index].priority_offset;
                return g_ew2a[action_index].action;
next:
                action_index++;
        }
        return ACTION_NONE;
}
