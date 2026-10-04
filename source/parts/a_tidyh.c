/*
 * parts/a_tidyh.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */

/* ACTION_TIDY_HOUSE (also run during the move-in cutscene).  The
   resident walks to the top-floor filing cabinet, turns to the screen
   and rummages in it with a_watat, which opens it if it is shut.  He
   closes it again (a_opcfc) when a 0..100 roll beats his
   initiative_threshold, and always during the cutscene (introSeq).
   Interrupted on the way, he simply gives up. */
void
a_tidyh()
{
        /* The walk call is tested inline, with no local for it. */

        hs_posXY(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
        if (lcp_wkD() != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_watat();

        /* Both call results are used in place; adding a local here
           would change the compiled code. */
        if (lcp.initiative_threshold < rndRng(0, 100) ||
            introSeq != NO)
                a_opcfc();
}
