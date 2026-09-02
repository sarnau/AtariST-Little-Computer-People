/*
 * parts/tv_scrc.c -- shared body; LCP_ORG links it in tvanim.c,
 * LCP_STX in the 0xdece object (0x13074, right after a_playc).  Files under parts/
 * are never compiled standalone.
 */

#ifdef FAITHFUL
void
tv_scrc()
{
        RECT16          rect;
        unsigned short  rnd;

        rect.x1 = 293; rect.y1 =  99;
        rect.x2 = 308; rect.y2 = 106;

        sc_sdtb();
        v_bar(vdihnd, &rect.x1);
        sc_sdtf();
        gameTick(1);

        rnd = (unsigned short) Random();
        if ((rnd & 1) == 0)
                tv_boul();
        else
                tv_patl();
}
#else   /* STX: link #-24 -- a 10-short point buffer, the random
           draw consumed in place, and the arms the other way round. */

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
#endif
