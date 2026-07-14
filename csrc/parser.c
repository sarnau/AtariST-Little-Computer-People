/*
 * parser.c -- the NLP command parser.
 *
 * The 1985 parser is a bitmask-based bag-of-words matcher:
 *   1. Uppercase and split the input into whitespace-delimited words
 *      via command_upperstr.
 *   2. For each word, look it up in valid_word_table[] via
 *      check_valid_word_input, which returns a WORD_ID.  Unrecognised
 *      words nudge the priority up (making the command less likely to
 *      actually fire, since higher = deprioritised).
 *   3. For each recognised WORD_ID, set one bit in the appropriate
 *      position slot of g_ewb[] using the two lookup
 *      tables word__entered_to_position[] (which of the 10 bytes) and
 *      g_ew2b[] (which of the 8 bits).
 *   4. Walk g_ew2a[] in order.  Each entry is a per-
 *      position bitmask; a row matches if every bit set in
 *      entry.table[i] is also set in g_ewb[i], for all
 *      i in 0..9.  First match wins.  Table is terminated by a row
 *      whose table[0] byte is 0xff.
 *
 * addr: check_entered_command(), command_upperstr(),
 *       check_valid_word_input(), lcp_toupper()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern PLAYER   lcp;                            /* the resident LCP */
extern short    g_aprio;
extern unsigned char    g_ewb[];
extern char             _user_input_buffer[];
extern short            _happiness_to_priority[];
extern char *           valid_word_table[];
extern short            word__entered_to_position[];
extern short            g_ew2b[];
extern unsigned char    _bitmask_1_2_4_8_10_20_40_80_0[];
extern WORD_TO_ACTION   g_ew2a[];
extern short    randomRange();                  /* random.c */
extern short    randomRange();

/* lcp_toupper: single-char ASCII uppercase.  Returns the char unchanged
   if it isn't in [a..z].
   addr: lcp_toupper() */

short
lcp_toupper(ch)
short   ch;
{
        if (ch > 0x60 && ch < 0x7b)
                ch = ch - 0x20;
        return ch;
}

/* command_upperstr: tokenize + uppercase.  Skips leading non-letters,
   then copies an uppercase word into `dest` and returns a pointer to
   the character right after the word (typically the delimiter).
   Returns NULL when no more letters remain in the input.
   addr: command_upperstr() */

char *
command_upperstr(str, dest)
char *  str;
char *  dest;
{
        short   c;

        for (;;) {
                c = lcp_toupper((short) *str);
                if (c == 0)
                        return (char *) 0;
                if (c > 0x40 && c < 0x5b)
                        break;
                str = str + 1;
        }
        for (;;) {
                c = lcp_toupper((short) *str);
                if (!(c > 0x40 && c < 0x5b))
                        break;
                *dest = (char) c;
                dest = dest + 1;
                str = str + 1;
        }
        *dest = '\0';
        return str;
}

/* check_valid_word_input: look up `word` in the vocabulary table.
   Returns the (0-indexed) WORD_ID on hit, or WORD_NONE on miss.  The
   dictionary walk stops at the first NULL pointer (table sentinel).
   The 1985 code had a hard cap at index 9998 and stepped the index by
   WORD_PLEASE (=1); preserved verbatim.
   addr: check_valid_word_input() */

short
check_valid_word_input(word)
char *  word;
{
        char *  dict_ptr;
        char *  input_ptr;
        short   word_index;

        word_index = WORD_NONE + 1;             /* start at 0 */
        while (word_index <= 9998) {
                dict_ptr = valid_word_table[word_index];
                if (dict_ptr == (char *) 0)
                        return WORD_NONE;

                input_ptr = word;
                while (*input_ptr == *dict_ptr) {
                        if (*input_ptr == 0)
                                return word_index;
                        input_ptr = input_ptr + 1;
                        dict_ptr  = dict_ptr  + 1;
                }
                word_index = word_index + WORD_PLEASE;
        }
        return WORD_NONE;
}

/* check_entered_command: full parse.
   addr: check_entered_command() */

short
check_entered_command(str)
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
        rnd = randomRange(0, 3);
        g_aprio = _happiness_to_priority[lcp.happiness] + rnd;

        /* Tokenize and mask-accumulate. */
        while ((str = command_upperstr(str, _user_input_buffer)) !=
               (char *) 0) {
                entered_word = check_valid_word_input(_user_input_buffer);
                if (entered_word == WORD_NONE) {
                        /* Unrecognised word -- +4 priority penalty. */
                        g_aprio = g_aprio + 4;
                } else if (entered_word > 0) {
                        short   pos = word__entered_to_position[entered_word];
                        short   bit = g_ew2b[entered_word];
                        g_ewb[pos] |=
                                _bitmask_1_2_4_8_10_20_40_80_0[bit];
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
