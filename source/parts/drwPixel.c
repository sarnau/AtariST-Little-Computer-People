/*
 * parts/drwPixel.c -- included by stx_u2.c at its place in the object's function order;
 * never compiled on its own.
 */
/* Plots one pixel as a degenerate two-point polyline (start == end),
   since the VDI bindings used here have no single-pixel call. */

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
