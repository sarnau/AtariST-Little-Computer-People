/*
 * parts/vswr_mode.c -- shared body.  vdistx.c includes it at its LCP_STX
 * position in the binding module.
 * Files under parts/ are never compiled standalone.
 */

void
vswr_mode(handle, mode)
short   handle;
short   mode;
{
        intin[0]  = mode;
        contrl[0] = 32;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
