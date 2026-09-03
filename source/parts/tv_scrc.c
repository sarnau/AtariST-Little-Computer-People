/*
 * parts/tv_scrc.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x13074, right after a_playc). Files under parts/ are never
 * compiled standalone.
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
