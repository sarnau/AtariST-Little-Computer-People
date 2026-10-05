/*
 * Included by stx_u3.c; never compiled on its own.
 */
/* Look one uppercased word up in the vocabulary.  vwd_tab is scanned
   from the front and the index of the FIRST exact match is returned,
   so a spelling listed twice can only ever yield its earlier entry;
   WORD_NONE comes back when the table's NULL end is reached.  Called
   by matchCommand for every word of a typed command. */
short
lookupWord(word)
char *  word;
{
        /* The scanned character gets a short of its own, and the
           function simply falls out of the loop -- the missing trailing
           `return WORD_NONE` is deliberate (the original has none). */
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
