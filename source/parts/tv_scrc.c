/*
 * parts/tv_scrc.c -- shared body; LCP_ORG links it in tvanim.c,
 * LCP_STX in the 0xdece object (0x13074, right after a_playc).  Files under parts/
 * are never compiled standalone.
 */

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
