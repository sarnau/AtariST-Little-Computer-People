/* Plots one pixel as a degenerate two-point polyline (start == end),
   since the VDI bindings used here have no single-pixel call. */

void
drawPixel(x, y, color)
short   x;
short   y;
short   color;
{
        short   pts[4];

        beginDraw();
        vsl_color(vdiHandle, colorPens[color]);
        pts[0] = x;
        pts[1] = y;
        pts[2] = x;
        pts[3] = y;
        v_pline(vdiHandle, 2, pts);
        endDraw();
}
