/* VDI: sets the fill interior style (hollow, solid, pattern, hatch) on
   workstation handle and returns the style the VDI selected. */
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
