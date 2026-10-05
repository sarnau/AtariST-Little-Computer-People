/* VDI: sets the writing mode (replace, transparent, XOR, reverse
   transparent) on workstation handle and returns the mode selected. */
void
vswr_mode(handle, mode)
short   handle;
short   mode;
{
        intin[0]  = mode;
        contrl[0] = VDI_VSWR_MODE;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
