/*
 * parts/vswr_mode.c -- shared body.  the VDI binding module is ordered
 * differently in the two revisions, so vdistx.c includes these in
 * LCP_STX order and vdiown.c in LCP_ORG's.
 * Files under parts/ are never compiled standalone.
 */

void
vswr_mode(handle, mode)
short   handle;
short   mode;
{
#ifdef FAITHFUL
        contrl[0] = 32;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = mode;
        vdi_go();
#else
        intin[0]  = mode;
        contrl[0] = 32;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
#endif
}
