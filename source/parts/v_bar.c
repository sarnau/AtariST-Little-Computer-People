/*
 * parts/v_bar.c -- shared body.  vdistx.c includes it at its LCP_STX
 * position in the binding module.
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
        vdipb[2]  = pxy;
        contrl[0] = 11;
        contrl[1] = 2;
        contrl[3] = 0;
        contrl[5] = 1;
        contrl[6] = handle;
        vdi_go();
        vdipb[2]  = ptsin;
}
