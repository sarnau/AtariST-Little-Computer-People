/*
 * parts/plErCol.c -- LCP_STX only; the games object carries it at
 * 0x871a, immediately after plEr.  Files under parts/ are never
 * compiled standalone.
 */

/* plErCol: plEr with an explicit fill colour.  Unlike plEr it does not
   go through initVdi/exitVdi -- the four attribute calls are written
   out here.
   addr: 0x871a */

void
plErCol(x1, y1, x2, y2, color)
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
        vswr_mode(vdihnd, 1);
        vsf_interior(vdihnd, 2);
        vsf_style(vdihnd, 8);
        vsf_color(vdihnd, vdi_colt[color]);
        v_bar(vdihnd, rect);
}
