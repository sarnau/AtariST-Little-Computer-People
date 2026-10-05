/*
 * parts/vdiClear.c -- the second half of vdiInit: reset the fill
 * attributes, hide the mouse and bar the whole screen.  It must
 * directly follow parts/vdiInit.c so vdiInit's call to it stays a
 * short branch.
 * Included by stx_u1.c; never compiled on its own.
 */
void
vdiClear()
{
        short   rect[4];

        vswr_mode(vdihnd, MD_REPLACE);
        vsf_interior(vdihnd, FIS_PATTERN);
        vsf_style(vdihnd, FILL_SOLID);
        vsf_color(vdihnd, 0);
        rect[0] = 0;
        rect[1] = 0;
        if (scr_scal == 2) {
                rect[2] = 639;
                rect[3] = 399;
        } else {
                rect[2] = 319;
                rect[3] = 199;
        }
        graf_mouse(M_OFF, 0L);
        v_bar(vdihnd, rect);
        vsf_color(vdihnd, 1);
}
