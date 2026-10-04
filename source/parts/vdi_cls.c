/*
 * parts/vdi_cls.c -- LCP_STX 0x66fe, the second half of vdi_init:
 * reset the fill attributes, hide the mouse and bar the whole screen.
 * Must be linked immediately after parts/vdi_init.c (bsr.s).
 * Files under parts/ are never compiled standalone.
 */
void
vdi_cls()
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
