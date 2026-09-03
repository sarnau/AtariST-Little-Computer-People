/*
 * vdistx.c -- Activision's VDI binding module as LCP_STX links it.
 *
 * The two revisions carry the SAME bindings in a different order, and
 * LCP_STX's module is bigger: it also holds vqt_attributes and
 * vst_height (copied verbatim from the DRI VDIBIND sources rather than
 * pulled from the library), and v_opnvwk / vro_cpyfm live here too
 * instead of in a separate workstation object.  Its addresses:
 *
 *     vswr_mode 0x1733a < v_bar 0x17374 < v_gtext 0x173ba
 *     < v_opnvwk 0x17426 < v_pline 0x174a6 < vqt_attributes 0x174e4
 *     < vro_cpyfm 0x1753a < vsf_color 0x17596 < vsf_interior 0x175d0
 *     < vsf_style 0x1760a < vsl_color 0x17644 < vst_color 0x1767e
 *     < vst_height 0x176b8
 *
 * There is also just ONE trap dispatcher and ONE parameter block:
 * every binding here reaches gsx1 (vdistx_a.s, 0x1772e, right behind
 * wr_src/wr_dst) and every one of them aims `vdipb` -- LCP_ORG's
 * vdi_go/vdipb and vdilib.c's vdi_go2/vdipb2 are two extra copies of
 * the same 22-byte routine, which is the whole of the port's former
 * +44-byte text surplus.  vdiown.h maps both names onto gsx1 for this
 * configuration.
 *
 * tools/stx_units.txt makes the default build compile this file and
 * skip vdiown.c and vdilib.c; FAITHFUL does the reverse.
 */

#ifndef FAITHFUL

#include "types.h"
#include "globals.h"
#include "vdiown.h"

extern short *  vdipb[];
extern void     wr_src();       /* vdistx_a.s: contrl[7..8]  = long */
extern void     wr_dst();       /* vdistx_a.s: contrl[9..10] = long */

#include "parts/vswr_mode.c"    /* 0x1733a */
#include "parts/v_bar.c"        /* 0x17374 */
#include "parts/v_gtext.c"      /* 0x173ba */

/* v_opnvwk (0x17426): points the block's intin/intout/ptsout entries
   at the caller's arrays for the call, then restores all four. */
void
v_opnvwk(work_in, handle, work_out)
short * work_in;
short * handle;
short * work_out;
{
        vdipb[1] = work_in;
        vdipb[3] = work_out;
        vdipb[4] = (short *) ((long) work_out + 90);
        contrl[0] = 100;
        contrl[1] = 0;
        contrl[3] = 11;
        contrl[6] = *handle;
        vdi_go();
        *handle = contrl[6];
        vdipb[1] = intin;
        vdipb[3] = intout;
        vdipb[4] = ptsout;
        vdipb[2] = ptsin;
}

#include "parts/v_pline.c"      /* 0x174a6 */

/* vqt_attributes (0x174e4): the DRI VDIBIND body, aiming the block's
   intout/ptsout entries at the caller's 12+ shorts. */
void
vqt_attributes(handle, attrib)
short   handle;
short * attrib;
{
        vdipb[3] = attrib;
        vdipb[4] = (short *) ((long) attrib + 12);
        contrl[0] = 38;
        contrl[1] = 0;
        contrl[3] = 0;
        contrl[6] = handle;
        vdi_go();
        vdipb[3] = intout;
        vdipb[4] = ptsout;
}

/* vro_cpyfm (0x1753a): the array-pxy blit. */
void
vro_cpyfm(handle, mode, pxy, src, dst)
short   handle;
short   mode;
short * pxy;
long    src;
long    dst;
{
        intin[0] = mode;
        wr_src(src);
        wr_dst(dst);
        vdipb[2] = pxy;
        contrl[0] = 109;
        contrl[1] = 4;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        vdipb[2] = ptsin;
}

#include "parts/vsf_color.c"    /* 0x17596 */
#include "parts/vsf_interior.c" /* 0x175d0 */
#include "parts/vsf_style.c"    /* 0x1760a */
#include "parts/vsl_color.c"    /* 0x17644 */
#include "parts/vst_color.c"    /* 0x1767e */

/* vst_height (0x176b8): the DRI VDIBIND body. */
void
vst_height(handle, height, char_w, char_h, cell_w, cell_h)
short   handle;
short   height;
short * char_w;
short * char_h;
short * cell_w;
short * cell_h;
{
        ptsin[0] = 0;
        ptsin[1] = height;
        contrl[0] = 12;
        contrl[1] = 1;
        contrl[3] = 0;
        contrl[6] = handle;
        vdi_go();
        *char_w = ptsout[0];
        *char_h = ptsout[1];
        *cell_w = ptsout[2];
        *cell_h = ptsout[3];
}

#endif  /* !FAITHFUL */
