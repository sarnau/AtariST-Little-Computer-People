/*
 * parser.c -- the NLP command parser.
 *
 * The 1985 parser is a bitmask-based bag-of-words matcher:
 *   1. Uppercase and split the input into whitespace-delimited words
 *      via cmd_upp.
 *   2. For each word, look it up in vwd_tab[] via
 *      chk_vwd, which returns a WORD_ID.  Unrecognised
 *      words nudge the priority up (making the command less likely to
 *      actually fire, since higher = deprioritised).
 *   3. For each recognised WORD_ID, set one bit in the appropriate
 *      position slot of g_ewb[] using the two lookup
 *      tables ew2pos[] (which of the 10 bytes) and
 *      g_ew2b[] (which of the 8 bits).
 *   4. Walk g_ew2a[] in order.  Each entry is a per-
 *      position bitmask; a row matches if every bit set in
 *      entry.table[i] is also set in g_ewb[i], for all
 *      i in 0..9.  First match wins.  Table is terminated by a row
 *      whose table[0] byte is 0xff.
 *
 * addr: chk_encm(), cmd_upp(),
 *       chk_vwd(), lcp_upp()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "parser.h"
#include "random.h"
#include "vocab.h"

/* lcp_upp: single-char ASCII uppercase.  Returns the char unchanged
   if it isn't in [a..z].
   addr: lcp_upp() */

short
lcp_upp(ch)
short   ch;
{
        if (ch > 0x60 && ch < 0x7b)
                ch = ch - 0x20;
        return ch;
}

/* cmd_upp: tokenize + uppercase.  Skips leading non-letters,
   then copies an uppercase word into `dest` and returns a pointer to
   the character right after the word (typically the delimiter).
   Returns NULL when no more letters remain in the input.
   addr: cmd_upp() */

char *
cmd_upp(str, dest)
char *  str;
char *  dest;
{
        short   c;

        for (;;) {
                c = lcp_upp((short) *str);
                if (c == 0)
                        return (char *) 0;
                if (c > 0x40 && c < 0x5b)
                        break;
                str = str + 1;
        }
        for (;;) {
                c = lcp_upp((short) *str);
                if (!(c > 0x40 && c < 0x5b))
                        break;
                *dest = (char) c;
                dest = dest + 1;
                str = str + 1;
        }
        *dest = '\0';
        return str;
}

/* chk_vwd: look up `word` in the vocabulary table.
   Returns the (0-indexed) WORD_ID on hit, or WORD_NONE on miss.  The
   dictionary walk stops at the first NULL pointer (table sentinel).
   The 1985 code had a hard cap at index 9998 and stepped the index by
   WORD_PLEASE (=1); preserved verbatim.
   addr: chk_vwd() */

short
chk_vwd(word)
char *  word;
{
        char *  dict_ptr;
        char *  input_ptr;
        short   word_index;

        word_index = WORD_NONE + 1;             /* start at 0 */
        while (word_index <= 9998) {
                dict_ptr = vwd_tab[word_index];
                if (dict_ptr == (char *) 0)
                        return WORD_NONE;

                input_ptr = word;
                while (*input_ptr == *dict_ptr) {
                        if (*input_ptr == 0)
                                return word_index;
                        input_ptr = input_ptr + 1;
                        dict_ptr  = dict_ptr  + 1;
                }
                word_index = word_index + 1;
        }
        return WORD_NONE;
}

/* chk_encm: full parse.
   addr: chk_encm() */

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
