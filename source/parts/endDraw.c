/*
 * Included by stx_u2.c; never compiled on its own.
 */

/* Ends a beginDraw bracket: restores the logical screen saved in
   drawLogbase. */
void
endDraw()
{
        Setscreen(drawLogbase, (void *)-1L, -1);
}
