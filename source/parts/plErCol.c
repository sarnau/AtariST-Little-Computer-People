/*
 * parts/plErCol.c -- included by games.c immediately after plEr; never
 * compiled on its own.
 */

/* plEr with an explicit fill colour.  Unlike plEr it does not go
   through initVdi/exitVdi -- the four attribute calls are written out
   here. */

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
        vswr_mode(vdihnd, MD_REPLACE);
        vsf_interior(vdihnd, FIS_PATTERN);
        vsf_style(vdihnd, FILL_SOLID);
        vsf_color(vdihnd, vdi_colt[color]);
        v_bar(vdihnd, rect);
}
