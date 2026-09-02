/*
 * parts/fl_ltpl.c -- shared body; LCP_ORG links it in letload.c,
 * LCP_STX in the 0xdece object (0x648c, just before cpyScr).  Files under parts/
 * are never compiled standalone.
 */
/* addr: fl_ltpl() */
void
fl_ltpl()
{
        /* STX's frame is -12: an unused short ahead of linecount, then
           the walking pointer. */
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
                   sites, not a do/while's one. */
                i++;
                while (*i >= ' ')
                        i++;

                /* Skip the terminator run. */
                while (*i < ' ')
                        i++;
        }
}
