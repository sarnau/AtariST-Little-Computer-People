/*
 * parts/vst_color.c -- shared body.  vdistx.c includes it at its LCP_STX
 * position in the binding module.
 * Files under parts/ are never compiled standalone.
 */

void
vst_color(handle, index)
short   handle;
short   index;
{
        intin[0]  = index;
        contrl[0] = 22;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
