/*
 * parts/v_pline.c -- included by vdistx.c, whose include order is the
 * binding module's function order; never compiled on its own.
 */

/* VDI polyline: draws count points from pxy (x,y pairs) on
   workstation handle, in the current line colour. */
void
v_pline(handle, count, pxy)
short   handle;
short   count;
short * pxy;
{
        /* Aim the parameter block's ptsin pointer at the caller's
           points for the call instead of copying them, then restore
           it. */
        vdipb[2]  = pxy;
        contrl[0] = VDI_V_PLINE;
        contrl[1] = count;
        contrl[3] = 0;
        contrl[6] = handle;
        vdi_go();
        vdipb[2]  = ptsin;
}
