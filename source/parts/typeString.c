/* Types the string str through typeChar, word by word, wrapping to a
   new line before any word that would pass column 40.  val is an
   indent: a negative value types -val spaces first, and a positive one
   does so only when the line already has text on it.  Each word is
   collected in letterWord before it is typed.  Returns the character that
   ended the scan (the string's terminator in practice). */
short
typeString(str, val)
char *  str;
short   val;
{
        /* The letterWord index is declared first (declaration order fixes
           the frame layout) and there is no NULL guard.  Both scan
           loops are `while ((ch = *str++) <op> ' ')`, which Alcyon
           compiles by saving the flags across the pointer increment;
           keep that form. */
        short   i;
        short   wordLength;
        short   ch;
        BOOL16  wordWrapNeeded;

        if (val < 0 || typedCursor > 0) {
                if (val < 0)
                        val = -val;
                for (i = 0; i < val; i++)
                        typeChar(' ');
        }

        wordWrapNeeded = NO;
        while (wordWrapNeeded == NO) {
                /* Skip inter-word spaces (emit if line already started),
                   then step back onto the first non-space. */
                while ((ch = *str++) == ' ')
                        if (typedCursor > 0)
                                typeChar(ch);
                str--;

                i = 0;
                while ((ch = *str++) > ' ') {
                        /* Index first, on purpose: `*(i + letterWord)`
                           compiles differently from letterWord[i]. */
                        *(i + letterWord) = ch;
                        i++;
                }
                if (ch != ' ')
                        wordWrapNeeded = YES;
                else
                        str--;

                /* Word-wrap at 40 columns. */
                if (typedCursor + i > 39)
                        typeChar(13);

                for (wordLength = 0; wordLength < i; wordLength++) {
                        ch = letterWord[wordLength];
                        typeChar(ch);
                }
        }
        return ch;
}
