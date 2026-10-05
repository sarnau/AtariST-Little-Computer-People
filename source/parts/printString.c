/*
 * Draws a string character by character through printChar, 8 pixels
 * apart.
 */

void
printString(str, x, y, color)
char *  str;
short   x;
short   y;
short   color;
{
        /* A short ch, with the fetch and step folded into the while
           condition: the original's shape. */
        short   ch;

        while ((ch = *str++) != 0) {
                printChar(ch, x, y, color);
                x += 8;
        }
}
