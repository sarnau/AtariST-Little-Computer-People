/*
 * Must sit right after a_playc.
 *
 * Included by stx_u2.c; never compiled on its own.
 */


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
