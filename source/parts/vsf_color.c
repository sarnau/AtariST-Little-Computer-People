/* VDI: sets the fill colour index on workstation handle and returns
   the index the VDI actually selected. */
void
vsf_color(handle, index)
short   handle;
short   index;
{
        intin[0]  = index;
        contrl[0] = VDI_VSF_COLOR;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
