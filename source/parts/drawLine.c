/* Draw one line from (x1,y1) to (x2,y2) into the house picture in
   game colour `color' (mapped to a VDI pen through colorPens).  The
   v_pline call is bracketed by beginDraw/endDraw so it lands on the
   back screen.  Used for the wall clock's hands and centre. */
void
drawLine(x1, y1, x2, y2, color)
short   x1;
short   y1;
short   x2;
short   y2;
short   color;
{
        short   pts[4];

        beginDraw();
        vsl_color(vdiHandle, colorPens[color]);
        pts[0] = x1;
        pts[1] = y1;
        pts[2] = x2;
        pts[3] = y2;
        v_pline(vdiHandle, 2, pts);
        endDraw();
}
