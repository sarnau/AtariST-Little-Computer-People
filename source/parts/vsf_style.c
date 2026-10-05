/* VDI: sets the fill style index (pattern or hatch number) on
   workstation handle and returns the style the VDI selected. */
void
vsf_style(handle, style)
short   handle;
short   style;
{
        intin[0]  = style;
        contrl[0] = VDI_VSF_STYLE;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
