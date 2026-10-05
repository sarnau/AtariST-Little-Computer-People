/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Ends a beginDraw bracket: restores the logical screen saved in
   g_srlgb. */
void
endDraw()
{
        Setscreen(g_srlgb, (void *)-1L, -1);
}
