/*
 * gfx_prim.c -- low-level graphics primitives above VDI/XBIOS.
 * addr: drwLine(), sc_sdtb(), sc_sdtf(), sc_firw(), blkcp32()
 */

#include "types.h"
#include "enums.h"
#include "structs.h"
#include <osbind.h>

#ifdef HOST

#include "hostgem.h"

#else

#include <vdibind.h>            /* v_pline, vsl_color, vsf_*, vst_*, vswr_mode, ... */

#endif
#include "obdefs1.h"
#include "gfx_prim.h"
#include "globals.h"
#include "sprender.h"

/* drwLine: single-segment line via VDI v_pline (backbuffer, restore).
   addr: drwLine() */


/* sc_sdtb: stash Logbase, Setscreen->g_srptr, reset fill to solid black.
   addr: sc_sdtb()

   Launcher note: launching via COMMAND.PRG leaves VDI in a state where
   vsl_color silently falls back to pen 15 (dark brown) -- water tank
   renders brown.  Launch LCP.PRG directly (GEM desktop or --auto). */

/* sc_sdtb -> parts/sc_sdtb.c (STX: 0xdece object, in the 0xdece object). */

/* sc_sdtf: restore log-base after sc_sdtb.
   addr: sc_sdtf() */

/* sc_sdtf -> parts/sc_sdtf.c (STX: 0xdece object, in the 0xdece object). */

/* sc_firw -> parts/sc_firw.c (STX: 0x16dcc, immediately after sc_sctd (bsr.s)). */

/* sc_firs/sc_firb -> parts/sc_firsb.c (STX: 0x16e22/0x16e76, in
   the 0x148fe object after sc_firw -- stx_u3.c includes them). */


/* initVdi: mini-game VDI setup.  Same shape as sc_sdtb but uses
   sv_lgb (nestable) and default fill = palette 0xC (light green).
   addr: initVdi() */


/* exitVdi: restore pre-mini-game log-base.
   addr: exitVdi() */


/* drwPixel -> parts/drwPixel.c (STX: 0x13930). */

/* blkcp32: unrolled 32-byte block copy (count * 32 bytes, 8 longs at a
   time -- MOVEM.L target).  Kept for byte-comparability.
   addr: blkcp32() */


/* cpyScr -> parts/cpyScr.c (STX: 0x64fa, in the 0x400c object). */

/* stpScrB (Ghidra 0x16576): init double-buffered compositing screen.
   1. MFDB_A.fd_addr=NULL (future vro_cpyfm read device screen).
   2. g_srptr = scrbufB + 0x12F, aligned UP to 512.
   3. Populate mf_scrp as scale*320 x scale*200; 0x1D00 = unused nplanes.
   4. cpyScr physbase snapshot for first compositing frame.
   addr: stpScrB() */

/* aes_init -> parts/aes_init.c (STX: 0x67aa, between vdi_cls and initBRev). */


#define REZ_ST_MEDIUM   1
#define REZ_ST_HIGH     2
#define M_OFF           256

/* vdi_init (ROM 0x7b72): open a virtual workstation on the physical
   handle using LOCAL work arrays, scr_scal=1, reset drawing
   attributes, hide the mouse, and clear the whole screen.  The ROM
   has no resolution check and no reboot alert. */


/* stpScrB -> parts/stpScrB.c (STX: 0x6576, in the 0x400c object). */

/* vst_h20: save VDI attrs to sv_vqta; set text height 20 px.
   addr: vdi_save_and_set_text_height_20() */

/* vst_h20 is a kept-only helper and lives in the games object in
   LCP_STX -- games.c includes parts/vst_h20.c. */

/* rst_vsth: restore VDI text height from sv_vqta[7] (cell height).
   addr: reset_vst_height() */

/* rst_vsth is a kept-only helper and lives in the games object in
   LCP_STX -- games.c includes parts/rst_vsth.c. */


/* moff/mon -> parts/moffmon.c (STX: 0xde36/0xde5c, at the head of
   the 0xdece object -- stx_u2.c includes them there). */


