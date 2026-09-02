/*
 * parts/drwLine.c -- shared body; LCP_ORG links it in init.c,
 * LCP_STX in the 0xdece object (0x138d4).  Files under parts/
 * are never compiled standalone.
 */

void
drwLine(x1, y1, x2, y2, color)
short   x1;
short   y1;
short   x2;
short   y2;
short   color;
{
        short   pts[4];

        sc_sdtb();
        vsl_color(vdihnd, vdi_colt[color]);
        pts[0] = x1;
        pts[1] = y1;
        pts[2] = x2;
        pts[3] = y2;
        v_pline(vdihnd, 2, pts);
        sc_sdtf();
}
