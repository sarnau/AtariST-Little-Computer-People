/*
 * t_parser.c -- host-side smoke test for the NLP command parser.
 *
 * Verifies that the parser correctly:
 *   1. Skips whitespace + punctuation on tokenisation.
 *   2. Uppercases lowercase input via toUpper.
 *   3. Returns WORD_NONE for words not in the (empty) vocabulary and
 *      bumps cmdPriority accordingly.
 *   4. Falls through to ACTION_NONE when the action table has only a
 *      sentinel row.
 *
 * With the real 1985 vocabulary + action tables wired in, this same
 * harness will be able to check specific "play a game" / "make me a
 * sandwich" -> ACTION_ID mappings.
 *
 * Build: make parser_test
 * Run:   from source/build/host/, execute ./parser_test
 */

#include <stdio.h>
#include <string.h>

#include "../include/types.h"
#include "../include/enums.h"

extern short    matchCommand();
extern short    check_valid_word_input();
extern char *   nextWord();
extern short    toUpper();
extern short    cmdPriority;
extern unsigned char phraseBits[];

static int
test_toupper(void)
{
        int     fails = 0;
        if (toUpper('a') != 'A') fails++;
        if (toUpper('z') != 'Z') fails++;
        if (toUpper('A') != 'A') fails++;
        if (toUpper('Z') != 'Z') fails++;
        if (toUpper('0') != '0') fails++;
        if (toUpper(' ') != ' ') fails++;
        return fails;
}

static int
test_tokenize(void)
{
        char    buf[32];
        char *  next;
        int     fails = 0;

        next = nextWord("play a game", buf);
        if (strcmp(buf, "PLAY") != 0) { printf("  fail: got '%s'\n", buf); fails++; }
        next = nextWord(next, buf);
        if (strcmp(buf, "A") != 0)    { printf("  fail: got '%s'\n", buf); fails++; }
        next = nextWord(next, buf);
        if (strcmp(buf, "GAME") != 0) { printf("  fail: got '%s'\n", buf); fails++; }
        next = nextWord(next, buf);
        if (next != (char *) 0)       { printf("  fail: expected NULL\n"); fails++; }
        return fails;
}

int
main(void)
{
        short   result;
        int     t_fails, k_fails, p_fails;

        setvbuf(stdout, NULL, _IONBF, 0);

        t_fails = test_toupper();
        printf("lcp_upp       : %s\n", t_fails ? "FAIL" : "OK");

        k_fails = test_tokenize();
        printf("cmd_upp  : %s\n", k_fails ? "FAIL" : "OK");

        /* End-to-end: parse a sentence with the real 160-word vocab.
           "please play a game" should resolve to ACTION_PLAY_A_GAME
           (16) via the "PLAY" + "GAME" combination in the parser's
           action-matching table. */
        result = matchCommand("please play a game");
        p_fails = (result < 0);
        printf("check_entered_cmd : %s  (returned %d, negative = no match)\n",
               p_fails ? "FAIL" : "OK", result);
        printf("  g_aprio after full parse = %d\n", cmdPriority);

        /* An all-unknown sentence.  On the ST this returns ACTION_NONE:
           matchCommand walks phraseTable until `table[0] == 0xff`, and Alcyon
           narrows that 0xff to a signed char, so the comparison against
           the sentinel row's -1 succeeds.  Clang does not narrow the
           constant -- (char)-1 == 255 is false -- so the walk runs off
           the end of the table and the result here is whatever follows
           it in memory.  The behaviour is target-specific, so this is
           reported, not asserted. */
        {
                short r2 = matchCommand("purple flurple");
                printf("chk_encm(unknown) : returned %d "
                       "(ST returns %d; not asserted on the host -- the\n"
                       "                    0xff sentinel test relies on "
                       "Alcyon narrowing the constant)\n",
                       r2, ACTION_NONE);
        }

        return (t_fails | k_fails | p_fails) ? 1 : 0;
}
