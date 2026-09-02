/*
 * parts/vdi_init.c -- LCP_STX splits the port's vdi_init in two:
 * this half (0x6680) opens the virtual workstation through global
 * work arrays, refuses anything but low resolution, and tail-calls
 * the attribute/clear half (parts/vdi_cls.c, 0x66fe, which must sit
 * immediately after it -- the call is a bsr.s).  LCP_ORG has neither
 * the split nor the resolution check, and keeps its own vdi_init in
 * gfx_prim.c.  Files under parts/ are never compiled standalone.
 */
void
vdi_init()
{
        short   i;

        vdihnd = vdi_hnd;
        for (i = 0; i < 10; i++)
                work_in[i] = 1;
        work_in[10] = 2;
        v_opnvwk(work_in, &vdihnd, wk_out);
        scr_scal = 1;
        if (wk_out[0] > 600)
                while (1)
                        form_alert(0,
                                "[1][Must be in|low resolution.][REBOOT]");
        vdi_cls();
}
