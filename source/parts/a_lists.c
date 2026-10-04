/*
 * parts/a_lists.c -- included by stx_u2.c; never compiled on its own.
 */
/* a_lists: pick a random .sng file and start it playing.
   Uses lcp_food as a modulo index (the 1985 code reused the field). */

void
a_lists()
{
        /* Three locals: a temporary, index (reused as the '.' scan
           counter) and the name pointer. */
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
        Fsfirst("*.sng", F_NORMAL);
        while (--index != 0)
                Fsnext();
        filename = ((_DTA *) Fgetdta())->d_fname;
        for (index = 0; filename[index] != '.'; index++)
                ;
        filename[index + 4] = '\0';
        sgPlay(filename);
}
