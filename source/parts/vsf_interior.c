/*
 * Position in vdistx.c is the binding module's original order.
 *
 * Included by vdistx.c; never compiled on its own.
 */

void
vsf_interior(handle, style)
short   handle;
short   style;
{
        intin[0]  = style;
        contrl[0] = VDI_VSF_INTERIOR;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
