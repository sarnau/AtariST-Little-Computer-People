/*
 * vdistx.c -- Activision's VDI binding module.
 *
 * It holds the game's own bindings plus vqt_attributes and vst_height
 * (copied from the DRI VDIBIND sources rather than pulled from the
 * library), and v_opnvwk / vro_cpyfm, instead of a separate
 * workstation object.  The order of the bindings and #include lines
 * below is the module's function order in the original and must not
 * change:
 *
 *     vswr_mode < v_bar < v_gtext < v_opnvwk < v_pline
 *     < vqt_attributes < vro_cpyfm < vsf_color < vsf_interior
 *     < vsf_style < vsl_color < vst_color < vst_height
 *
 * There is just ONE trap dispatcher and ONE parameter block: every
 * binding here reaches gsx1 (vdistx_a.s, right behind wr_src/wr_dst)
 * and every one of them aims `vdipb`.  vdiown.h maps the vdi_go /
 * vdi_go2 spellings onto gsx1.
 *
 * This is the port's only VDI binding module.
 */

#include "types.h"
#include "globals.h"
#include "vdiown.h"

extern short *  vdipb[];
extern void     wr_src();       /* vdistx_a.s: contrl[7..8]  = long */
extern void     wr_dst();       /* vdistx_a.s: contrl[9..10] = long */

#include "parts/vswr_mode.c"
#include "parts/v_bar.c"
#include "parts/v_gtext.c"

/* v_opnvwk points the block's intin/intout/ptsout entries
   at the caller's arrays for the call, then restores all four. */
void
v_opnvwk(workIn, handle, workOut)
short * workIn;
short * handle;
short * workOut;
{
        vdipb[1] = workIn;
        vdipb[3] = workOut;
        vdipb[4] = (short *) ((long) workOut + 90);
        contrl[0] = VDI_V_OPNVWK;
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

#include "parts/v_pline.c"

/* vqt_attributes: the DRI VDIBIND body, aiming the block's
   intout/ptsout entries at the caller's 12+ shorts. */
void
vqt_attributes(handle, attrib)
short   handle;
short * attrib;
{
        vdipb[3] = attrib;
        vdipb[4] = (short *) ((long) attrib + 12);
        contrl[0] = VDI_VQT_ATTRIBUTES;
        contrl[1] = 0;
        contrl[3] = 0;
        contrl[6] = handle;
        vdi_go();
        vdipb[3] = intout;
        vdipb[4] = ptsout;
}

/* vro_cpyfm: the array-pxy blit.  Copies the rectangle pxy[0..3] of
   the source MFDB to pxy[4..7] of the destination MFDB with writing
   mode `mode` (VDI opaque raster copy).  The parameter block's ptsin
   entry is aimed at the caller's pxy for the trap instead of copying
   the eight points, then restored; src and dst are MFDB addresses
   that wr_src/wr_dst store into contrl. */
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
        contrl[0] = VDI_VRO_CPYFM;
        contrl[1] = 4;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        vdipb[2] = ptsin;
}

#include "parts/vsf_color.c"
#include "parts/vsf_interior.c"
#include "parts/vsf_style.c"
#include "parts/vsl_color.c"
#include "parts/vst_color.c"

/* vst_height: the DRI VDIBIND body.  Sets the text character height
   in pixels for workstation `handle` and returns the resulting
   character width/height and cell width/height through the four
   pointers. */
void
vst_height(handle, height, charW, charH, cellW, cellH)
short   handle;
short   height;
short * charW;
short * charH;
short * cellW;
short * cellH;
{
        ptsin[0] = 0;
        ptsin[1] = height;
        contrl[0] = VDI_VST_HEIGHT;
        contrl[1] = 1;
        contrl[3] = 0;
        contrl[6] = handle;
        vdi_go();
        *charW = ptsout[0];
        *charH = ptsout[1];
        *cellW = ptsout[2];
        *cellH = ptsout[3];
}

