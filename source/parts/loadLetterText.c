/*
 * Loads letter.txt and builds the line-pointer table letterLines.
 */
void
loadLetterText()
{
        /* The unused short ahead of linecount must stay: removing it
           changes the compiled code. */
        short   unused;
        short   linecount;
        char *  i;

        unpackFile("letter.txt",
                             (unsigned char *) letterText,
                             10496);

        i = letterText;
        for (linecount = 0; linecount < 360; linecount++) {
                letterLines[linecount] = i;

                /* Step once, then a plain `while` -- two increment
                   sites, not a do/while's one, as in the original. */
                i++;
                while (*i >= ' ')
                        i++;

                /* Skip the terminator run. */
                while (*i < ' ')
                        i++;
        }
}
