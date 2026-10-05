/*
 * Matches a typed command against the phraseTable action table.
 */
short
matchCommand(str)
char *  str;
{
        /* No `rnd` temporary, and the priority seed adds the roll
           FIRST: both are the original's shape. */
        short   i;
        short   row;
        short   enteredWord;

        /* Clear the accumulated position/bit mask. */
        for (i = 0; i < 10; i++)
                phraseBits[i] = 0;

        /* Seed the priority from happiness + a small random nudge. */
        cmdPriority = rndRng(0, 3) + moodPriority[resident.happiness];

        /* Tokenize and mask-accumulate, breaking out of a `while (1)`
           as the original does. */
        while (1) {
                if ((str = nextWord(str, cmdWord)) == (char *) 0)
                        break;
                /* The "unrecognised" sentinel tested here is 0, even
                   though lookupWord returns -1 when it runs off the table.
                   So the word at index 0 (PLEASE) never contributes its
                   bit and takes the +4 penalty instead.  1985 behaviour,
                   kept on purpose. */
                if ((enteredWord = lookupWord(cmdWord)) == 0) {
                        /* Unrecognised word -- +4 priority penalty. */
                        cmdPriority += 4;
                } else if (enteredWord > 0) {
                        /* Both index tables are char[], and there
                           are no temporaries. */
                        phraseBits[wordByte[enteredWord]] |=
                                bitMask8[wordBit[enteredWord]];
                }
        }

        /* Walk the action-matching table until a row matches or we hit
           the 0xff sentinel.  The walk is a `while (1)` whose sentinel
           test breaks to the ACTION_NONE return placed after the loop;
           a row that fails jumps straight to the increment through an
           explicit goto, not a break plus an `i >= 10` re-test. */
        row = 0;
        while (1) {
                if (phraseTable[row].table[0] == EW2A_END)
                        break;
                for (i = 0; i < 10; i++)
                        if ((phraseTable[row].table[i] & phraseBits[i]) !=
                            phraseTable[row].table[i])
                                goto next;
                cmdPriority += phraseTable[row].priority_offset;
                return phraseTable[row].action;
next:
                row++;
        }
        return ACTION_NONE;
}
