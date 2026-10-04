/*
 * parts/fl_ltpl.c -- included by stx_u1.c; never compiled on its own.
 * Loads letter.txt and builds the line-pointer table g_ltlp.
 */
void
fl_ltpl()
{
        /* The unused short ahead of linecount must stay: removing it
           changes the compiled code. */
        short   unused;
        short   linecount;
        char *  i;

        fr_reac("letter.txt",
                             (unsigned char *) g_lttx,
                             10496);

        i = g_lttx;
        for (linecount = 0; linecount < 360; linecount++) {
                g_ltlp[linecount] = i;

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
