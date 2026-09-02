/*
 * parts/strPr.c -- shared body; LCP_ORG links it in renderx.c,
 * LCP_STX in the 0xdece object (0x16ea8, immediately before prCh).  Files under parts/
 * are never compiled standalone.
 */

void
strPr(str, x, y, color)
char *  str;
short   x;
short   y;
short   color;
{
#ifdef FAITHFUL
        char    ch;

        for (;;) {
                ch = *str;
                str = str + 1;
                if (ch == 0)
                        break;
                prCh((short) ch, x, y, color);
                x = x + 8;
        }
#else
        /* STX: a short ch, the fetch/step folded into the while
           condition, and x stepped in the argument slot. */
        short   ch;

        while ((ch = *str++) != 0) {
                prCh(ch, x, y, color);
                x += 8;
        }
#endif
}
