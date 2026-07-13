/*
 * vdi.c -- GEM VDI wrappers.
 *
 * Every wrapper here follows the standard Alcyon C GEM binding shape:
 *
 *   1. Fill the shared contrl[] parameter block with:
 *        contrl[0]  opcode (function number)
 *        contrl[1]  ptsin count      (number of pt pairs)
 *        contrl[3]  intin count      (number of int params)
 *        contrl[5]  sub-opcode       (for v_bar/v_fillarea/vro_cpyfm)
 *        contrl[6]  vdi handle
 *   2. Fill intin[]/ptsin[] with the actual arguments.
 *   3. Issue trap #2 with D1 pointing at vdipb (the 5-pointer struct
 *      that points at contrl/intin/ptsin/intout/ptsout).
 *
 * On the host we can't run trap #2, so vdi_call() is a no-op stub in
 * this same file; on the ST target Alcyon's <vdibind.h> provides an
 * assembly stub that fires the trap.  Either way, the caller-visible
 * behaviour of each wrapper matches Alcyon's stock <vdibind.h>
 * implementation exactly.
 *
 * VDI opcode reference (subset actually used by LCP):
 *   v_pline         6      polyline draw
 *   v_gtext         8      graphic text
 *   v_bar          11.1    filled bar (sub-op 1 of v_fillarea)
 *   vsl_color      17      set line color
 *   vst_color      22      set text color
 *   vsf_interior   23      set fill interior
 *   vsf_style      24      set fill style
 *   vsf_color      25      set fill color
 *   vswr_mode      32      set writing mode
 *   vro_cpyfm     109      raster copy (masked blit)
 *
 * The wrappers are one-liners under Alcyon; the actual game-visible
 * content is all in the parameter setup.
 */

#include "types.h"
#include "structs.h"
#include "globals.h"

/* Trap #2 dispatcher.  On the ST it's the standard AES/VDI entry point;
   here it's declared as an external function that on the target maps
   to Alcyon's assembler `crystal` (or similar) stub, and on the host
   is defined below as a no-op. */
extern void     vdi_call();

#ifdef HOST
/* Host: no trap, no observable effect.  The parameter arrays are still
   populated by the wrappers below so a test harness can inspect them
   to verify each call built the right VDI parameter block. */
void
vdi_call()
{
        /* no-op on host */
}
#else
/* Alcyon target: fire the GEM VDI trap.  Convention (from Alcyon's
   own gemlib/vdi.c, which shipped with the Atari ST Developer Kit):

     d1 = &vdipb          (5-pointer parameter block)
     d0 = 115             (VDI dispatch sub-opcode inside trap #2)
     trap #2              (AES/VDI/BDOS gateway)

   Alcyon prepends an underscore to C symbols at the asm level, so the
   `vdipb` array is `_vdipb` in the asm operand.  The three-statement
   sequence matches Alcyon gemlib byte-for-byte. */
void
vdi_call()
{
        asm("        move.l  #_vdipb,d1");
        asm("        moveq.l #115,d0");
        asm("        trap    #2");
}
#endif

/* vsl_color: set the line-drawing colour (VDI opcode 17). */

void
vsl_color(handle, color)
short   handle;
short   color;
{
        contrl[0] = 17;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = color;
        vdi_call();
}

/* vst_color: set the text-drawing colour (VDI opcode 22). */

void
vst_color(handle, color)
short   handle;
short   color;
{
        contrl[0] = 22;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = color;
        vdi_call();
}

/* vsf_color: set the fill (interior area) colour (VDI opcode 25). */

void
vsf_color(handle, color)
short   handle;
short   color;
{
        contrl[0] = 25;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = color;
        vdi_call();
}

/* vsf_interior: set the fill-interior type (0=hollow, 1=solid,
   2=pattern, 3=hatch, 4=user-defined).  VDI opcode 23. */

void
vsf_interior(handle, style)
short   handle;
short   style;
{
        contrl[0] = 23;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = style;
        vdi_call();
}

/* vsf_style: set the fill-style (pattern index within the current
   interior).  VDI opcode 24. */

void
vsf_style(handle, style)
short   handle;
short   style;
{
        contrl[0] = 24;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = style;
        vdi_call();
}

/* vswr_mode: set the writing mode (1=replace, 2=transparent,
   3=XOR, 4=reverse-transparent).  VDI opcode 32. */

void
vswr_mode(handle, mode)
short   handle;
short   mode;
{
        contrl[0] = 32;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = mode;
        vdi_call();
}

/* v_pline: draw a polyline through `n` (x, y) points.  The points
   array is a flat short[2*n] laid out as (x0, y0, x1, y1, ...).
   VDI opcode 6. */

void
v_pline(handle, n, pts)
short   handle;
short   n;
short * pts;
{
        short   i;

        contrl[0] = 6;
        contrl[1] = n;                  /* ptsin count */
        contrl[3] = 0;                  /* intin count */
        contrl[6] = handle;
        for (i = 0; i < n * 2; i = i + 1)
                ptsin[i] = pts[i];
        vdi_call();
}

/* v_gtext: draw an ASCII string at (x, y).  VDI opcode 8.  ptsin
   holds the anchor point; intin holds the character codes. */

void
v_gtext(handle, x, y, str)
short   handle;
short   x;
short   y;
char *  str;
{
        short   len;

        len = 0;
        while (str[len] != 0) {
                intin[len] = (short) (unsigned char) str[len];
                len = len + 1;
        }
        contrl[0] = 8;
        contrl[1] = 1;                  /* 1 pt pair */
        contrl[3] = len;
        contrl[6] = handle;
        ptsin[0]  = x;
        ptsin[1]  = y;
        vdi_call();
}

/* v_bar: filled bar (sub-opcode 1 of v_fillarea, opcode 11).  Two
   corner points define the rectangle. */

void
v_bar(handle, pxy)
short   handle;
short * pxy;
{
        contrl[0] = 11;
        contrl[1] = 2;                  /* 2 pt pairs */
        contrl[3] = 0;
        contrl[5] = 1;                  /* sub-opcode 1 = v_bar */
        contrl[6] = handle;
        ptsin[0]  = pxy[0];
        ptsin[1]  = pxy[1];
        ptsin[2]  = pxy[2];
        ptsin[3]  = pxy[3];
        vdi_call();
}

/* vdi_copy_rect (vro_cpyfm): raster copy with a mode.  VDI opcode 109.
   Copies from `src` MFDB to `dst` MFDB with the specified raster op
   (S_ONLY=3, NOTS_AND_D=4, S_XOR_D=6, etc.).  The 8-value pxy array
   in ptsin[] specifies source (sx1,sy1,sx2,sy2) and destination
   (dx1,dy1,dx2,dy2) rectangles.

   contrl[7..10] carry the source and destination MFDB pointers as
   two 16-bit halves each (high word first, low word second) -- this
   is how the 68000 VDI ABI stashes 32-bit pointers into a 16-bit
   parameter block. */

void
vdi_copy_rect(handle, mode, src, dst,
              sx1, sy1, sx2, sy2,
              dx1, dy1, dx2, dy2)
short   handle;
short   mode;
MFDB *  src;
MFDB *  dst;
short   sx1;
short   sy1;
short   sx2;
short   sy2;
short   dx1;
short   dy1;
short   dx2;
short   dy2;
{
        contrl[0] = 109;
        contrl[1] = 4;                  /* 4 pt pairs = 8 shorts */
        contrl[3] = 1;
        contrl[6] = handle;
        contrl[7]  = (short) (((long) src >> 16) & 0xffff);
        contrl[8]  = (short) ((long) src & 0xffff);
        contrl[9]  = (short) (((long) dst >> 16) & 0xffff);
        contrl[10] = (short) ((long) dst & 0xffff);

        intin[0] = mode;

        ptsin[0] = sx1;  ptsin[1] = sy1;
        ptsin[2] = sx2;  ptsin[3] = sy2;
        ptsin[4] = dx1;  ptsin[5] = dy1;
        ptsin[6] = dx2;  ptsin[7] = dy2;

        vdi_call();
}
