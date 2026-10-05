/*
 * parts/carryInFront.c -- included by stx_u2.c; never compiled on its own.
 */

/* Same as carryBehind but in the in-front-of-LCP layer. */

void
carryInFront(g_seix)
short   g_seix;
{
        /* No local for the slot (as in activateSprite): g_seslm[] is
           re-read at every use. */
        g_selaf[g_seix] = SPRITE_IN_FRONT;
        layoutSlots();
        g_seaim[g_seslm[g_seix]]  = g_sedim[g_seix];
        g_seams[g_seslm[g_seix]]   = g_sedms[g_seix];
        g_seach[g_seslm[g_seix]] = g_sedeh[g_seix];
        g_seacw[g_seslm[g_seix]]  = g_sedew[g_seix];
        g_lcyof = YES;
        g_lcieo       = g_seix;
}
