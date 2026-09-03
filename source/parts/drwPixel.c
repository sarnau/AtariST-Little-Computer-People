/*
 * parts/drwPixel.c -- shared body; LCP_STX links it in the 0xdece
 * object (0x13930). Files under parts/ are never compiled standalone.
 */
/* drwPixel: single-pixel via degenerate v_pline (VDI single-px fast path).
   addr: draw_pixel @ 0x23930 */

void
drwPixel(x, y, color)
short   x;
short   y;
short   color;
{
        short   pts[4];

        sc_sdtb();
        vsl_color(vdihnd, vdi_colt[color]);
        pts[0] = x;
        pts[1] = y;
        pts[2] = x;
        pts[3] = y;
        v_pline(vdihnd, 2, pts);
        sc_sdtf();
}
