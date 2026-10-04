/*
 * parts/lt_tysa.c -- included by stx_u2.c; never compiled on its own.
 */

/* Types the string str through lt_tyca, word by word, wrapping to a
   new line before any word that would pass column 40.  val is an
   indent: a negative value types -val spaces first, and a positive one
   does so only when the line already has text on it.  Each word is
   collected in g_ltscb before it is typed.  Returns the character that
   ended the scan (the string's terminator in practice). */
short
lt_tysa(str, val)
char *  str;
short   val;
{
        /* The g_ltscb index is declared first (declaration order fixes
           the frame layout) and there is no NULL guard.  Both scan
           loops are `while ((ch = *str++) <op> ' ')`, which Alcyon
           compiles by saving the flags across the pointer increment;
           keep that form. */
        short   i;
        short   word_length;
        short   ch;
        BOOL16  word_wrap_needed;

        if (val < 0 || g_cdibp > 0) {
                if (val < 0)
                        val = -val;
                for (i = 0; i < val; i++)
                        lt_tyca(' ');
        }

        word_wrap_needed = NO;
        while (word_wrap_needed == NO) {
                /* Skip inter-word spaces (emit if line already started),
                   then step back onto the first non-space. */
                while ((ch = *str++) == ' ')
                        if (g_cdibp > 0)
                                lt_tyca(ch);
                str--;

                i = 0;
                while ((ch = *str++) > ' ') {
                        /* Index first, on purpose: `*(i + g_ltscb)`
                           compiles differently from g_ltscb[i]. */
                        *(i + g_ltscb) = ch;
                        i++;
                }
                if (ch != ' ')
                        word_wrap_needed = YES;
                else
                        str--;

                /* Word-wrap at 40 columns. */
                if (g_cdibp + i > 39)
                        lt_tyca(13);

                for (word_length = 0; word_length < i; word_length++) {
                        ch = g_ltscb[word_length];
                        lt_tyca(ch);
                }
        }
        return ch;
}
