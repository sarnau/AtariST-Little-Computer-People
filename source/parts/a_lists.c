/*
 * parts/a_lists.c -- shared body; LCP_ORG links it in aleisure.c,
 * LCP_STX in the 0xdece object (0x1398c, right after drwPixel).  Files under parts/
 * are never compiled standalone.
 */
/* a_lists: pick a random .sng file and start it playing.
   Uses lcp_food as a modulo index (1985 code reused the field).
   addr: a_lists() */

void
a_lists()
{
#ifdef FAITHFUL
        short   result;
        short   index;
        _DTA *   dta_ptr;
        char *  filename;
        short   i;
#else
        /* STX: link #-12 -- a temporary, index (reused as the '.'
           scan counter) and the name pointer. */
        short   tmp;
        short   index;
        char *  filename;
#endif

        if (lcp_recP != NO)
                return;

        hs_posXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
#ifdef FAITHFUL
        result = lcp_wkD();
        if (result != 0)
#else
        if (lcp_wkD() != 0)
#endif
                return;

        gameTick(2);
        li_loor();
        lcp_recP = YES;

#ifdef FAITHFUL
        index = rndRng(0, lcp_food - 1) + 1;
        Fsfirst("*.sng", 0L);
        while ((index = index - 1) != 0)
                Fsnext();
        dta_ptr = (_DTA *) Fgetdta();
        filename = dta_ptr->d_fname;
        for (i = 0; filename[i] != '.'; i = i + 1)
                ;
        filename[i + 4] = '\0';
#else
        tmp = rndRng(0, lcp_food - 1);
        index = tmp + 1;
        Fsfirst("*.sng", 0);
        while (--index != 0)
                Fsnext();
        filename = ((_DTA *) Fgetdta())->d_fname;
        for (index = 0; filename[index] != '.'; index++)
                ;
        filename[index + 4] = '\0';
#endif
        sgPlay(filename);
}
