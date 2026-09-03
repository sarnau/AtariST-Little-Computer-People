/*
 * parts/a_lists.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x1398c, right after drwPixel). Files under parts/ are never
 * compiled standalone.
 */
/* a_lists: pick a random .sng file and start it playing.
   Uses lcp_food as a modulo index (1985 code reused the field).
   addr: a_lists() */

void
a_lists()
{
        /* STX: link #-12 -- a temporary, index (reused as the '.'
           scan counter) and the name pointer. */
        short   tmp;
        short   index;
        char *  filename;

        if (lcp_recP != NO)
                return;

        hs_posXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        gameTick(2);
        li_loor();
        lcp_recP = YES;

        tmp = rndRng(0, lcp_food - 1);
        index = tmp + 1;
        Fsfirst("*.sng", 0);
        while (--index != 0)
                Fsnext();
        filename = ((_DTA *) Fgetdta())->d_fname;
        for (index = 0; filename[index] != '.'; index++)
                ;
        filename[index + 4] = '\0';
        sgPlay(filename);
}
