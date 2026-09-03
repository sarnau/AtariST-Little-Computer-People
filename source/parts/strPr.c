/*
 * parts/strPr.c -- shared body; LCP_STX links it in the 0xdece object
 * (0x16ea8, immediately before prCh). Files under parts/ are never
 * compiled standalone.
 */

void
strPr(str, x, y, color)
char *  str;
short   x;
short   y;
short   color;
{
        /* STX: a short ch, the fetch/step folded into the while
           condition, and x stepped in the argument slot. */
        short   ch;

        while ((ch = *str++) != 0) {
                prCh(ch, x, y, color);
                x += 8;
        }
}
