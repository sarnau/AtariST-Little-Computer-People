/* The second half of vdiInit: reset the fill attributes, hide the
   mouse and bar the whole screen. It must directly follow
   parts/vdiInit.c so vdiInit's call to it stays a short branch. */
void
vdiClear()
{
        short   rect[4];

        vswr_mode(vdiHandle, MD_REPLACE);
        vsf_interior(vdiHandle, FIS_PATTERN);
        vsf_style(vdiHandle, FILL_SOLID);
        vsf_color(vdiHandle, 0);
        rect[0] = 0;
        rect[1] = 0;
        if (screenScale == 2) {
                rect[2] = 639;
                rect[3] = 399;
        } else {
                rect[2] = 319;
                rect[3] = 199;
        }
        graf_mouse(M_OFF, 0L);
        v_bar(vdiHandle, rect);
        vsf_color(vdiHandle, 1);
}
