/*
 * parts/vsl_color.c -- included by vdistx.c at its place in the VDI
 * binding module; never compiled on its own.
 */

/* VDI: sets the polyline colour index on workstation handle and
   returns the index the VDI actually selected. */
void
vsl_color(handle, index)
short   handle;
short   index;
{
        /* intin is set before contrl, and the binding RETURNS
           intout[0]: the original binding's shape. */
        intin[0]  = index;
        contrl[0] = VDI_VSL_COLOR;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
