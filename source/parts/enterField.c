/*
 * parts/enterField.c -- included by stx_u1.c, between titleScreen and eraseChar;
 * never compiled on its own.
 */

/* enterField: read a fixed-width numeric field on the title screen.
   `tmpl` is both the prompt drawn into the field and the character
   restored by backspace, so every third column (the separator in
   MM/DD/YY and HH:MM) is skipped over rather than typed into.  The
   digits land in inputLine as values, not characters. */

void
enterField(x, y, tmpl, len, color)
short   x;
short   y;
char *  tmpl;
short   len;
short   color;
{
        short   i;
        short   ch;

        eraseRectColor(x, y - 7, (len << 3) + x, y, 15);
        printString(tmpl, x, y, color);
        i = 0;
        while (1) {
                ch = getKey();
                if (ch == 8 && i > 0) {
                        i--;
                        if (i % 3 == 2)
                                i--;
                        eraseChar((i << 3) + x, y, 15);
                        printChar(tmpl[i], (i << 3) + x, y, color);
                        continue;
                }
                if (ch < '0')
                        continue;
                if (ch > '9')
                        continue;
                eraseChar((i << 3) + x, y, 15);
                printChar(ch, (i << 3) + x, y, color);
                *(i + inputLine) = ch - '0';
                i++;
                if (i % 3 == 2)
                        i++;
                if (i >= len)
                        break;
        }
}
