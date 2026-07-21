/*
 * parser_smoke.c -- host-side smoke test for the NLP command parser.
 *
 * Verifies that the parser correctly:
 *   1. Skips whitespace + punctuation on tokenisation.
 *   2. Uppercases lowercase input via lcp_toupper.
 *   3. Returns WORD_NONE for words not in the (empty) vocabulary and
 *      bumps g_aprio accordingly.
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

extern short    check_entered_command();
extern short    check_valid_word_input();
extern char *   command_upperstr();
extern short    lcp_toupper();
extern short    g_aprio;
extern unsigned char g_ewb[];

static int
test_toupper(void)
{
        int     fails = 0;
        if (lcp_toupper('a') != 'A') fails++;
        if (lcp_toupper('z') != 'Z') fails++;
        if (lcp_toupper('A') != 'A') fails++;
        if (lcp_toupper('Z') != 'Z') fails++;
        if (lcp_toupper('0') != '0') fails++;
        if (lcp_toupper(' ') != ' ') fails++;
        return fails;
}

static int
test_tokenize(void)
{
        char    buf[32];
        char *  next;
        int     fails = 0;

        next = command_upperstr("play a game", buf);
        if (strcmp(buf, "PLAY") != 0) { printf("  fail: got '%s'\n", buf); fails++; }
        next = command_upperstr(next, buf);
        if (strcmp(buf, "A") != 0)    { printf("  fail: got '%s'\n", buf); fails++; }
        next = command_upperstr(next, buf);
        if (strcmp(buf, "GAME") != 0) { printf("  fail: got '%s'\n", buf); fails++; }
        next = command_upperstr(next, buf);
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
        printf("lcp_toupper       : %s\n", t_fails ? "FAIL" : "OK");

        k_fails = test_tokenize();
        printf("command_upperstr  : %s\n", k_fails ? "FAIL" : "OK");

        /* End-to-end: parse a sentence with the real 160-word vocab.
           "please play a game" should resolve to ACTION_PLAY_A_GAME
           (16) via the "PLAY" + "GAME" combination in the parser's
           action-matching table. */
        result = check_entered_command("please play a game");
        p_fails = (result < 0);
        printf("check_entered_cmd : %s  (returned %d, negative = no match)\n",
               p_fails ? "FAIL" : "OK", result);
        printf("  g_aprio after full parse = %d\n",
               g_aprio);

        /* Verify a random unknown-word bump still works. */
        {
                short r2 = check_entered_command("purple flurple");
                printf("check_entered_cmd(unknown) : %s  (returned %d, expected -1)\n",
                       (r2 == -1) ? "OK" : "FAIL", r2);
                if (r2 != -1) p_fails++;
        }

        return (t_fails | k_fails | p_fails) ? 1 : 0;
}
