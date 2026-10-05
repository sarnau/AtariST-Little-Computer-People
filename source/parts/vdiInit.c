/*
 * the first half of vdiInit: opens the virtual workstation through the
 * global work arrays, refuses anything but low resolution, and then
 * calls the attribute/clear half (parts/vdiClear.c), which must
 * directly follow it.
 */
void
vdiInit()
{
        short   i;

        vdiHandle = physHandle;
        for (i = 0; i < 10; i++)
                work_in[i] = 1;
        work_in[10] = 2;
        v_opnvwk(work_in, &vdiHandle, wk_out);
        screenScale = 1;
        if (wk_out[0] > 600)
                while (1)
                        form_alert(ALERT_NO_DEFAULT,
                                "[1][Must be in|low resolution.][REBOOT]");
        vdiClear();
}
