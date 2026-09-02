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

/* chk_encm -> parts/chk_encm.c (STX: 0x16f9a, in the 0x148fe object between prCh and prsCmd). */
#ifdef FAITHFUL
#include "parts/chk_encm.c"
#endif
