/*
 * parts/vsf_style.c -- included by vdistx.c, whose include order is the
 * binding module's function order; never compiled on its own.
 */

void
vsf_style(handle, style)
short   handle;
short   style;
{
        intin[0]  = style;
        contrl[0] = VDI_VSF_STYLE;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
