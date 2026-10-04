/*
 * Must sit right after a_playc.
 *
 * Included by stx_u2.c; never compiled on its own.
 */


/* Clears the small screen at (293,99)-(308,106) to colour 0, waits a
   tick, then plays one of its two random animations -- pattern lines
   (tv_patl) or a bouncing dot (tv_boul).  Used for the rare "clear the
   screen" gesture while the resident plays on the computer (a_playc). */
void
tv_scrc()
{
        short   pts[10];

        pts[0] = 293; pts[1] =  99;
        pts[2] = 308; pts[3] = 106;

        sc_sdtb();
        v_bar(vdihnd, pts);
        sc_sdtf();
        gameTick(1);

        if ((Random() & 1) != 0)
                tv_patl();
        else
                tv_boul();
}
