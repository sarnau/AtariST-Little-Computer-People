/*
 * parts/v_pline.c -- shared body.  vdistx.c includes it at its LCP_STX
 * position in the binding module.
 * Files under parts/ are never compiled standalone.
 */

void
v_pline(handle, count, pxy)
short   handle;
short   count;
short * pxy;
{
        /* STX aims the parameter block at the caller's points. */
        vdipb[2]  = pxy;
        contrl[0] = 6;
        contrl[1] = count;
        contrl[3] = 0;
        contrl[6] = handle;
        vdi_go();
        vdipb[2]  = ptsin;
}
