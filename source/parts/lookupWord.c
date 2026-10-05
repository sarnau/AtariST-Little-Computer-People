/* Look one uppercased word up in the vocabulary.  vocabulary is scanned
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
        short   wordIndex;
        short   c;
        char *  dictPtr;
        char *  inputPtr;

        for (wordIndex = 0; wordIndex < 9999; wordIndex++) {
                dictPtr = vocabulary[wordIndex];
                if (dictPtr == (char *) 0)
                        return WORD_NONE;

                inputPtr = word;
                while ((c = *inputPtr++) == *dictPtr++) {
                        if (c == 0)
                                return wordIndex;
                }
        }
}
