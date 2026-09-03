/*
 * parts/vsf_style.c -- shared body.  vdistx.c includes it at its LCP_STX
 * position in the binding module.
 * Files under parts/ are never compiled standalone.
 */

void
vsf_style(handle, style)
short   handle;
short   style;
{
        intin[0]  = style;
        contrl[0] = 24;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
