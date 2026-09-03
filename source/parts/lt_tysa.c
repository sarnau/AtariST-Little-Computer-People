/*
 * parts/lt_tysa.c -- shared body.  LCP_STX places it at its own
 * address inside the 0xdece object, far from the port's other
 * aletter functions, so stx_u2.c includes it in LCP_STX order.
 * Files under parts/ are never compiled standalone.
 */

short
lt_tysa(str, val)
char *  str;
short   val;
{
        /* STX declares the g_ltscb index first and has no NULL guard;
           both scan loops are `while ((ch = *str++) <op> ' ')`, which
           Alcyon compiles by saving the flags across the pointer
           increment. */
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
                        /* Index first: Alcyon folds the base into the
                           address register (add.l #g_ltscb,a1). */
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
