/*
 * vdiown.c -- the game's own minimal VDI bindings (ROM 0xd664..0xd9xx).
 *
 * The ROM does NOT route its drawing through the linked VDIBIND
 * library: it carries this private set of wrappers that write straight
 * into its own contrl/intin/ptsin arrays and trap into the VDI with
 * its own parameter block (vdi_go, hand assembly injected by
 * alcyon_build.sh -- c168 cannot emit trap #2).  These
 * definitions shadow the identically-named VDIBIND entries at link
 * time (game objects precede vdibind.a), exactly as in the ROM.
 * Byte-verified against LCP_ORG.PRG with verify_bytes.py.
 */

#include "types.h"
#include "globals.h"
#include "vdiown.h"

/* The VDI parameter block lives in globals.c (ROM data 0x12054). */
extern short *  vdipb[];

/* addr: vsl_color() (ROM 0xd676) */
void
vsl_color(handle, index)
short   handle;
short   index;
{
        /* STX assigns intin first and returns intout[0]; LCP_ORG
           fills contrl first and returns nothing. */
#ifdef FAITHFUL
        contrl[0] = 17;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = index;
        vdi_go();
#else
        intin[0]  = index;
        contrl[0] = 17;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
#endif
}

/* addr: vst_color() (ROM 0xd6a6) */
void
vst_color(handle, index)
short   handle;
short   index;
{
#ifdef FAITHFUL
        contrl[0] = 22;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = index;
        vdi_go();
#else
        intin[0]  = index;
        contrl[0] = 22;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
#endif
}

/* addr: vsf_color() (ROM 0xd6d6) */
void
vsf_color(handle, index)
short   handle;
short   index;
{
#ifdef FAITHFUL
        contrl[0] = 25;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = index;
        vdi_go();
#else
        intin[0]  = index;
        contrl[0] = 25;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
#endif
}

/* addr: vsf_interior() (ROM 0xd708) */
void
vsf_interior(handle, style)
short   handle;
short   style;
{
#ifdef FAITHFUL
        contrl[0] = 23;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = style;
        vdi_go();
#else
        intin[0]  = style;
        contrl[0] = 23;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
#endif
}

/* addr: vsf_style() (ROM 0xd73a) */
void
vsf_style(handle, style)
short   handle;
short   style;
{
#ifdef FAITHFUL
        contrl[0] = 24;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = style;
        vdi_go();
#else
        intin[0]  = style;
        contrl[0] = 24;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
#endif
}

/* addr: vswr_mode() (ROM 0xd76c) */
void
vswr_mode(handle, mode)
short   handle;
short   mode;
{
#ifdef FAITHFUL
        contrl[0] = 32;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        intin[0]  = mode;
        vdi_go();
#else
        intin[0]  = mode;
        contrl[0] = 32;
        contrl[1] = 0;
        contrl[3] = 1;
        contrl[6] = handle;
        vdi_go();
        return intout[0];
#endif
}

/* addr: v_pline() (ROM 0xd79e) */
void
v_pline(handle, count, pxy)
short   handle;
short   count;
short * pxy;
{
        short   i;

        contrl[0] = 6;
        contrl[1] = count;
        contrl[3] = 0;
        contrl[6] = handle;
        for (i = 0; count * 2 > i; i = i + 1)
                ptsin[i] = pxy[i];
        vdi_go();
}

/* addr: v_gtext() (ROM 0xd7fc) */
void
v_gtext(handle, x, y, str)
short   handle;
short   x;
short   y;
char *  str;
{
        short   i;

        for (i = 0; str[i] != 0; i = i + 1)
                intin[i] = str[i];
        contrl[0] = 8;
        contrl[1] = 1;
        contrl[3] = i;
        contrl[6] = handle;
        ptsin[0]  = x;
        ptsin[1]  = y;
        vdi_go();
}

/* addr: v_bar() (ROM 0xd872) */
void
v_bar(handle, pxy)
short   handle;
short * pxy;
{
        contrl[0] = 11;
        contrl[1] = 2;
        contrl[3] = 0;
        contrl[5] = 1;
        contrl[6] = handle;
        ptsin[0]  = pxy[0];
        ptsin[1]  = pxy[1];
        ptsin[2]  = pxy[2];
        ptsin[3]  = pxy[3];
        vdi_go();
}

/* addr: vroCpyD() (ROM 0xd8d2) -- discrete-argument vro_cpyfm. */
void
vroCpyD(handle, mode, src, dst, sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2)
short   handle;
short   mode;
long    src;
long    dst;
short   sx1;
short   sy1;
short   sx2;
short   sy2;
short   dx1;
short   dy1;
short   dx2;
short   dy2;
{
        contrl[0]  = 109;
        contrl[1]  = 4;
        contrl[3]  = 1;
        contrl[6]  = handle;
        contrl[7]  = (short) (src >> 16);
        contrl[8]  = (short) src;
        contrl[9]  = (short) (dst >> 16);
        contrl[10] = (short) dst;
        intin[0]   = mode;
        ptsin[0] = sx1;
        ptsin[1] = sy1;
        ptsin[2] = sx2;
        ptsin[3] = sy2;
        ptsin[4] = dx1;
        ptsin[5] = dy1;
        ptsin[6] = dx2;
        ptsin[7] = dy2;
        vdi_go();
}
