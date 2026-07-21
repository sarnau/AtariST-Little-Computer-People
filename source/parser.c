/*
 * parser.c -- the NLP command parser (bitmask bag-of-words matcher).
 * addr: chk_encm(), cmd_upp(), chk_vwd(), lcp_upp()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "parser.h"
#include "random.h"
#include "vocab.h"

/* addr: lcp_upp() */
short
lcp_upp(ch)
short   ch;
{
        if (ch > 0x60 && ch < 0x7b)
                ch = ch - 0x20;
        return ch;
}

/* addr: cmd_upp() */
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

/* addr: chk_vwd() */
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
