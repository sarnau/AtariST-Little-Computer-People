/*
 * Its position in vdistx.c's include list is the binding module's
 * layout and must not change.
 * Included by vdistx.c; never compiled on its own.
 */

void
vst_color(handle, index)
short   handle;
short   index;
{
        intin[0]  = index;
        contrl[0] = VDI_VST_COLOR;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
}
