/*
 * parts/chk_encm.c -- included by stx_u3.c; never compiled on its own.
 * Matches a typed command against the g_ew2a action table.
 */
short
chk_encm(str)
char *  str;
{
        /* No `rnd` temporary, and the priority seed adds the roll
           FIRST: both are the original's shape. */
        short   i;
        short   row;
        short   entered_word;

        /* Clear the accumulated position/bit mask. */
        for (i = 0; i < 10; i++)
                g_ewb[i] = 0;

        /* Seed the priority from happiness + a small random nudge. */
        g_aprio = rndRng(0, 3) + mood_pri[lcp.happiness];

        /* Tokenize and mask-accumulate, breaking out of a `while (1)`
           as the original does. */
        while (1) {
                if ((str = cmd_upp(str, usr_buf)) == (char *) 0)
                        break;
                /* The "unrecognised" sentinel tested here is 0, even
                   though chk_vwd returns -1 when it runs off the table.
                   So the word at index 0 (PLEASE) never contributes its
                   bit and takes the +4 penalty instead.  1985 behaviour,
                   kept on purpose. */
                if ((entered_word = chk_vwd(usr_buf)) == 0) {
                        /* Unrecognised word -- +4 priority penalty. */
                        g_aprio += 4;
                } else if (entered_word > 0) {
                        /* Both index tables are char[], and there
                           are no temporaries. */
                        g_ewb[ew2pos[entered_word]] |=
                                bm_lo[g_ew2b[entered_word]];
                }
        }

        /* Walk the action-matching table until a row matches or we hit
           the 0xff sentinel.  The walk is a `while (1)` whose sentinel
           test breaks to the ACTION_NONE return placed after the loop;
           a row that fails jumps straight to the increment through an
           explicit goto, not a break plus an `i >= 10` re-test. */
        row = 0;
        while (1) {
                if (g_ew2a[row].table[0] == EW2A_END)
                        break;
                for (i = 0; i < 10; i++)
                        if ((g_ew2a[row].table[i] & g_ewb[i]) !=
                            g_ew2a[row].table[i])
                                goto next;
                g_aprio += g_ew2a[row].priority_offset;
                return g_ew2a[row].action;
next:
                row++;
        }
        return ACTION_NONE;
}
