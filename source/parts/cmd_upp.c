/*
 * parts/cmd_upp.c -- shared body; LCP_ORG links it in parser.c,
 * LCP_STX in the 0xdece object (0x1711c, after chk_encm).  Files under parts/
 * are never compiled standalone.
 */
char *
cmd_upp(str, dest)
char *  str;
char *  dest;
{
        short   c;

        /* The fetch and the uppercase conversion are two statements,
           the bounds are inclusive, and the copy loop's terminator is
           written in the else arm (which also steps dest). */
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
