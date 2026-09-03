/*
 * parts/v_bar.c -- shared body.  the VDI binding module is ordered
 * differently in the two revisions, so vdistx.c includes these in
 * LCP_STX order and vdiown.c in LCP_ORG's.
 * Files under parts/ are never compiled standalone.
 */

void
v_bar(handle, pxy)
short   handle;
short * pxy;
{
        /* STX points the parameter block's ptsin entry at the
           caller's array for the duration of the call instead of
           copying the points, then restores it -- the same trick
           vdilib.c's vro_cpyfm uses. */
#ifdef FAITHFUL
        contrl[0] = 11;
        contrl[1] = 2;
        contrl[3] = 0;
        contrl[5] = 1;
        contrl[6] = handle;
        ptsin[0]  = pxy[0];
        ptsin[1]  = pxy[1];
        ptsin[2]  = pxy[2];
        ptsin[3]  = pxy[3];
        vdi_go();
#else
        vdipb[2]  = pxy;
        contrl[0] = 11;
        contrl[1] = 2;
        contrl[3] = 0;
        contrl[5] = 1;
        contrl[6] = handle;
        vdi_go();
        vdipb[2]  = ptsin;
#endif
}
