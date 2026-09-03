/*
 * parts/vsf_interior.c -- shared body.  vdistx.c includes it at its LCP_STX
 * position in the binding module.
 * Files under parts/ are never compiled standalone.
 */

void
vsf_interior(handle, style)
short   handle;
short   style;
{
        intin[0]  = style;
        contrl[0] = 23;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
