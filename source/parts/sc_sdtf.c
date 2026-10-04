/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Ends a sc_sdtb bracket: restores the logical screen saved in
   g_srlgb. */
void
sc_sdtf()
{
        Setscreen(g_srlgb, (void *)-1L, -1);
}
