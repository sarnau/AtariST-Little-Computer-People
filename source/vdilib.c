/*
 * vdilib.c -- the ROM's own workstation-open module (ROM 0xe754-0xe85a).
 *
 * Linked AFTER the game objects, in library position (see
 * alcyon_link.sh): Activision replaced the VDIBIND open/copy module
 * with their own, built around a second, runtime-patched VDI parameter
 * block (vdipb2, BSS) whose entries are pointed at caller arrays for
 * the duration of a call and then restored to the game's own
 * contrl/intin/ptsin/intout/ptsout set.  The three helpers written in
 * assembly by the original (wr_src / wr_dst / vdi_go2) are injected by
 * alcyon_build.sh.  These definitions shadow VDIBIND's entries.
 * Byte-verified against LCP_ORG.PRG.
 */

#include "types.h"
#include "globals.h"
#include "vdiown.h"

extern void     wr_src();       /* asm: contrl[7..8] = long arg  */
extern void     wr_dst();       /* asm: contrl[9..10] = long arg */
extern void     vdi_go2();      /* asm: trap #2 with vdipb2      */

short * vdipb2[5];

/* addr: v_opnvwk() (ROM 0xe754) */
void
v_opnvwk(work_in, handle, work_out)
short * work_in;
short * handle;
short * work_out;
{
        vdipb2[1] = work_in;
        vdipb2[3] = work_out;
        vdipb2[4] = (short *) ((long) work_out + 90);
        contrl[0] = 100;
        contrl[1] = 0;
        contrl[3] = 11;
        contrl[6] = *handle;
        vdi_go2();
        *handle = contrl[6];
        vdipb2[1] = intin;
        vdipb2[3] = intout;
        vdipb2[4] = ptsout;
        vdipb2[2] = ptsin;
}

/* addr: vro_cpyfm() (ROM 0xe7d4) -- the array-pxy blit: points the
   parameter block's ptsin entry at the caller's pxy for one call. */
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
        vdipb2[2] = pxy;
        contrl[0] = 109;
        contrl[1] = 4;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go2();
        vdipb2[2] = ptsin;
}
