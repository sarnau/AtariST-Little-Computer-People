/*
 * parts/chk_vwd.c -- shared body; LCP_ORG links it in parser.c,
 * LCP_STX in the 0xdece object (0x171ae, after cmd_upp).  Files under parts/
 * are never compiled standalone.
 */
/* addr: chk_vwd() */
short
chk_vwd(word)
char *  word;
{
        /* Frame -16: the scanned character gets a short of its own,
           and the function simply falls out of the loop -- there is no
           trailing `return WORD_NONE`. */
        short   word_index;
        short   c;
        char *  dict_ptr;
        char *  input_ptr;

        for (word_index = 0; word_index < 9999; word_index++) {
                dict_ptr = vwd_tab[word_index];
                if (dict_ptr == (char *) 0)
                        return WORD_NONE;

                input_ptr = word;
                while ((c = *input_ptr++) == *dict_ptr++) {
                        if (c == 0)
                                return word_index;
                }
        }
}
