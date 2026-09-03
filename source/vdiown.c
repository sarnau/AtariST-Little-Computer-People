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


/* addr: vswr_mode() (ROM 0xd76c) */
/* vswr_mode -> parts/vswr_mode.c (vdistx.c includes it in LCP_STX order). */
#ifdef FAITHFUL
#include "parts/vswr_mode.c"
#endif

/* addr: v_bar() (ROM 0xd872) */
/* v_bar -> parts/v_bar.c (vdistx.c includes it in LCP_STX order). */
#ifdef FAITHFUL
#include "parts/v_bar.c"
#endif

/* addr: v_gtext() (ROM 0xd7fc) */
/* v_gtext -> parts/v_gtext.c (vdistx.c includes it in LCP_STX order). */
#ifdef FAITHFUL
#include "parts/v_gtext.c"
#endif

/* addr: v_pline() (ROM 0xd79e) */
/* v_pline -> parts/v_pline.c (vdistx.c includes it in LCP_STX order). */
#ifdef FAITHFUL
#include "parts/v_pline.c"
#endif

/* addr: vsf_color() (ROM 0xd6d6) */
/* vsf_color -> parts/vsf_color.c (vdistx.c includes it in LCP_STX order). */
#ifdef FAITHFUL
#include "parts/vsf_color.c"
#endif

/* addr: vsf_interior() (ROM 0xd708) */
/* vsf_interior -> parts/vsf_interior.c (vdistx.c includes it in LCP_STX order). */
#ifdef FAITHFUL
#include "parts/vsf_interior.c"
#endif

/* addr: vsf_style() (ROM 0xd73a) */
/* vsf_style -> parts/vsf_style.c (vdistx.c includes it in LCP_STX order). */
#ifdef FAITHFUL
#include "parts/vsf_style.c"
#endif
/* addr: vsl_color() (ROM 0xd676) */
/* vsl_color -> parts/vsl_color.c (vdistx.c includes it in LCP_STX order). */
#ifdef FAITHFUL
#include "parts/vsl_color.c"
#endif

/* addr: vst_color() (ROM 0xd6a6) */
/* vst_color -> parts/vst_color.c (vdistx.c includes it in LCP_STX order). */
#ifdef FAITHFUL
#include "parts/vst_color.c"
#endif

/* vroCpyD -> parts/vroCpyD.c (STX: 0x63cc, in the 0x400c object
   between hs_posXY and al_loal -- stx_u1.c includes it there). */
#ifdef FAITHFUL
#include "parts/vroCpyD.c"
#endif
