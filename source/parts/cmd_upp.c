/*
 * Included by stx_u3.c; never compiled on its own.
 */
char *
cmd_upp(str, dest)
char *  str;
char *  dest;
{
        short   c;

        /* Skip to the first letter, then copy the word uppercased and
           NUL-terminate it.  The fetch and the conversion are two
           statements, and the terminator is written in the else arm
           (which also steps dest) -- the original's shape. */
        while (1) {
                c = *str;
                c = lcp_upp(c);
                if (c == 0)
                        return (char *) 0;
                if (c >= 'A' && c <= 'Z')
                        break;
                str++;
        }
        while (1) {
                c = *str;
                c = lcp_upp(c);
                if (c >= 'A' && c <= 'Z') {
                        *dest = c;
                        dest++;
                        str++;
                } else {
                        *dest = '\0';
                        dest++;
                        break;
                }
        }
        return str;
}
