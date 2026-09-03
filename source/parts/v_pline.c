/*
 * parts/v_pline.c -- shared body.  the VDI binding module is ordered
 * differently in the two revisions, so vdistx.c includes these in
 * LCP_STX order and vdiown.c in LCP_ORG's.
 * Files under parts/ are never compiled standalone.
 */

void
v_pline(handle, count, pxy)
short   handle;
short   count;
short * pxy;
{
#ifdef FAITHFUL
        short   i;

        contrl[0] = 6;
        contrl[1] = count;
        contrl[3] = 0;
        contrl[6] = handle;
        for (i = 0; count * 2 > i; i = i + 1)
                ptsin[i] = pxy[i];
        vdi_go();
#else
        /* STX aims the parameter block at the caller's points. */
        vdipb[2]  = pxy;
        contrl[0] = 6;
        contrl[1] = count;
        contrl[3] = 0;
        contrl[6] = handle;
        vdi_go();
        vdipb[2]  = ptsin;
#endif
}
