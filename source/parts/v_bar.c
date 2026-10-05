/* VDI filled rectangle (GDP bar) on workstation handle; pxy holds the
   two corners x1,y1,x2,y2. */
void
v_bar(handle, pxy)
short   handle;
short * pxy;
{
        /* Point the parameter block's ptsin entry at the caller's
           array for the duration of the call instead of copying the
           points, then restore it -- the same trick vro_cpyfm uses. */
        vdipb[2]  = pxy;
        contrl[0] = VDI_V_GDP;
        contrl[1] = 2;
        contrl[3] = 0;
        contrl[5] = GDP_BAR;
        contrl[6] = handle;
        vdi_go();
        vdipb[2] = ptsin;
}
