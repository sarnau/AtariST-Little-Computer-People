/*
 * Must sit right after useComputer.
 *
 * Included by stx_u2.c; never compiled on its own.
 */


/* Clears the small screen at (293,99)-(308,106) to colour 0, waits a
   tick, then plays one of its two random animations -- pattern lines
   (tvPattern) or a bouncing dot (tvBounce).  Used for the rare "clear the
   screen" gesture while the resident plays on the computer (useComputer). */
void
tvClearAnim()
{
        short   pts[10];

        pts[0] = 293; pts[1] =  99;
        pts[2] = 308; pts[3] = 106;

        beginDraw();
        v_bar(vdiHandle, pts);
        endDraw();
        gameTick(1);

        if ((Random() & 1) != 0)
                tvPattern();
        else
                tvBounce();
}
