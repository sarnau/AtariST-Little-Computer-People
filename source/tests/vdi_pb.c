/*
 * vdi_pb.c -- verify VDI wrappers build the correct parameter blocks.
 *
 * Since we can't actually issue trap #2 on the host, we test each
 * wrapper by calling it and inspecting the shared contrl[]/intin[]/
 * ptsin[] arrays afterward.  This proves the wrappers speak the right
 * VDI ABI to the trap dispatcher, regardless of whether the trap
 * itself fires.
 *
 * Build: make vdi_pb_test
 * Run:   from source/build/host/, execute ./vdi_pb_test
 */

#include <stdio.h>
#include "../include/types.h"
#include "../include/structs.h"

extern short    contrl[];
extern short    intin[];
extern short    ptsin[];
extern short    vdihandle;

extern void     vsl_color();
extern void     vst_color();
extern void     vsf_color();
extern void     vsf_interior();
extern void     vsf_style();
extern void     vswr_mode();
extern void     v_pline();
extern void     v_gtext();
extern void     v_bar();
extern void     vdi_copy_rect();

static int      fails;

static void
expect(name, got, want)
char *  name;
short   got;
short   want;
{
        if (got != want) {
                printf("  FAIL %s: got %d expected %d\n", name, got, want);
                fails = fails + 1;
        }
}

int
main()
{
        short   pts[4];
        MFDB    src, dst;

        setvbuf(stdout, NULL, _IONBF, 0);
        vdihandle = 42;

        vsl_color(vdihandle, 5);
        expect("vsl_color contrl[0]",   contrl[0],  17);
        expect("vsl_color contrl[3]",   contrl[3],   1);
        expect("vsl_color intin[0]",    intin[0],    5);

        vst_color(vdihandle, 7);
        expect("vst_color contrl[0]",   contrl[0],  22);
        expect("vst_color intin[0]",    intin[0],    7);

        vsf_color(vdihandle, 9);
        expect("vsf_color contrl[0]",   contrl[0],  25);
        expect("vsf_color intin[0]",    intin[0],    9);

        vsf_interior(vdihandle, 1);
        expect("vsf_interior contrl[0]",contrl[0],  23);
        expect("vsf_interior intin[0]", intin[0],    1);

        vsf_style(vdihandle, 3);
        expect("vsf_style contrl[0]",   contrl[0],  24);
        expect("vsf_style intin[0]",    intin[0],    3);

        vswr_mode(vdihandle, 2);
        expect("vswr_mode contrl[0]",   contrl[0],  32);
        expect("vswr_mode intin[0]",    intin[0],    2);

        pts[0] = 10; pts[1] = 20; pts[2] = 30; pts[3] = 40;
        v_pline(vdihandle, 2, pts);
        expect("v_pline contrl[0]",     contrl[0],   6);
        expect("v_pline contrl[1]",     contrl[1],   2);
        expect("v_pline ptsin[0]",      ptsin[0],   10);
        expect("v_pline ptsin[3]",      ptsin[3],   40);

        v_gtext(vdihandle, 100, 200, "Hi");
        expect("v_gtext contrl[0]",     contrl[0],   8);
        expect("v_gtext contrl[3]",     contrl[3],   2);
        expect("v_gtext ptsin[0]",      ptsin[0],  100);
        expect("v_gtext ptsin[1]",      ptsin[1],  200);
        expect("v_gtext intin[0]",      intin[0],  'H');
        expect("v_gtext intin[1]",      intin[1],  'i');

        pts[0] = 5; pts[1] = 6; pts[2] = 7; pts[3] = 8;
        v_bar(vdihandle, pts);
        expect("v_bar contrl[0]",       contrl[0],  11);
        expect("v_bar contrl[5] sub",   contrl[5],   1);
        expect("v_bar ptsin[3]",        ptsin[3],    8);

        src.fd_addr = (void *) 0x100000L;
        dst.fd_addr = (void *) 0x200000L;
        vdi_copy_rect(vdihandle, 3, &src, &dst,
                      0, 0, 15, 23,
                      100, 100, 115, 123);
        expect("vdi_copy_rect contrl[0]", contrl[0], 109);
        expect("vdi_copy_rect contrl[1]", contrl[1],   4);
        expect("vdi_copy_rect intin[0]",  intin[0],    3);
        expect("vdi_copy_rect ptsin[3]",  ptsin[3],   23);
        expect("vdi_copy_rect ptsin[7]",  ptsin[7],  123);

        if (fails == 0)
                printf("PASS: all 10 VDI wrappers build correct parameter blocks\n");
        return fails ? 1 : 0;
}
