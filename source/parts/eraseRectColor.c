/* panelErase with an explicit fill colour.  Unlike panelErase it does not go
   through panelBegin/panelEnd -- the four attribute calls are written out
   here. */

void
eraseRectColor(x1, y1, x2, y2, color)
short   x1;
short   y1;
short   x2;
short   y2;
short   color;
{
        short   rect[4];

        rect[0] = x1;
        rect[1] = y1;
        rect[2] = x2;
        rect[3] = y2;
        vswr_mode(vdiHandle, MD_REPLACE);
        vsf_interior(vdiHandle, FIS_PATTERN);
        vsf_style(vdiHandle, FILL_SOLID);
        vsf_color(vdiHandle, colorPens[color]);
        v_bar(vdiHandle, rect);
}
