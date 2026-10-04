/*
 * parts/strPr.c -- included by stx_u3.c; never compiled on its own.
 * Draws a string character by character through prCh, 8 pixels apart.
 */

void
strPr(str, x, y, color)
char *  str;
short   x;
short   y;
short   color;
{
        /* A short ch, with the fetch and step folded into the while
           condition: the original's shape. */
        short   ch;

        while ((ch = *str++) != 0) {
                prCh(ch, x, y, color);
                x += 8;
        }
}
