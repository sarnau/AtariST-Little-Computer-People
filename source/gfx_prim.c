/*
 * gfx_prim.c -- low-level graphics primitives above VDI/XBIOS.
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

/* The primitives that used to live here (drwLine, sc_sdtb, sc_sdtf,
   sc_firw, sc_firs/sc_firb, drwPixel, cpyScr, stpScrB, aes_init, moff/
   mon, vst_h20, rst_vsth) are in parts/ and are included by the unity
   units that match the original's object layout.

   Launcher note for sc_sdtb: launching via COMMAND.PRG leaves VDI in a
   state where vsl_color silently falls back to pen 15 (dark brown) --
   the water tank renders brown.  Launch LCP.PRG directly (GEM desktop
   or --auto).

   stpScrB sets up the double-buffered compositing screen:
   1. MFDB_A.fd_addr = NULL, so later vro_cpyfm calls read the device
      screen.
   2. g_srptr = scrbufB rounded UP to a 512-byte boundary.
   3. mf_scrp is filled in as scale*320 x scale*200.
   4. cpyScr takes a physbase snapshot for the first compositing frame.

   blkcp32 is hand assembly (blkcp_a.s): an unrolled copy of count * 32
   bytes, eight longs at a time. */


#define REZ_ST_MEDIUM   1
#define REZ_ST_HIGH     2
#define M_OFF           256

/* vdi_init opens a virtual workstation on the physical handle, sets
   scr_scal = 1, resets the drawing attributes, hides the mouse and
   clears the whole screen. */


/* vst_h20 saves the VDI text attributes to sv_vqta and sets a 20-pixel
   text height; rst_vsth restores the height from sv_vqta[7] (the cell
   height).  Both live in the games object, so games.c includes them. */
