/*
 * parts/vswr_mode.c -- included by vdistx.c at its place in the binding
 * module's order; never compiled on its own.
 */

void
vswr_mode(handle, mode)
short   handle;
short   mode;
{
        intin[0]  = mode;
        contrl[0] = VDI_VSWR_MODE;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
