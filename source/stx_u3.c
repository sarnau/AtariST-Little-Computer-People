/*
 * stx_u3.c -- unity unit for the sprite object: er_write, the sprite
 * engine, the compositor tick, keyboard dispatch and the parser.  See
 * stx_u1.c for the mechanism.
 *
 * The include list below IS the object's function order and must not
 * change.
 */


/* prCh needs obdefs.h (MD_TRANS/MD_REPLACE). */
#include "obdefs1.h"
#include "sprglobs.h"

/* deal_kc/putEv need the globals and prototypes their own files
   pull in. */
#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "gfx_prim.h"
#include "ai.h"
#include "events.h"
#include "keyboard.h"
#include "sound.h"
#include "render.h"
#include "renderx.h"
#include "parser.h"
#include "vocab.h"

#include "dat_u3a.c"


#include "alerts.c"
#include "sprites.c"
/* renderf.c straddles: sc_ren8 is in this object, ahead of
   lcp_hwt. */
#include "parts/sc_ren8.c"
/* lcp_hwt immediately precedes gameTick. */
#include "parts/lcp_hwt.c"
#include "tick.c"
/* Everything below follows gameTick, in this order. */
#include "parts/deal_kc.c"
#include "parts/p_dobls.c"
#include "parts/putEv.c"
#include "parts/getEv.c"
#include "parts/sp_draw.c"
#include "parts/sp_drin.c"
#include "parts/sp_updb.c"
#include "parts/sp_lcha.c"
#include "parts/sp_lchu.c"
/* sprites.c straddles within this object: these four sit past
   sp_lchu, not with sp_upds/sp_imfs at the front. */
#include "parts/sp_lbal.c"
#include "parts/sp_lbbd.c"
#include "parts/sp_lbhd.c"
#include "parts/sp_lcpf.c"
/* renderx.c and gfx_prim.c straddle: sc_sctd and sc_firw sit in
   this object, and sc_firw must directly follow sc_sctd so the call
   between them stays a short branch. */
#include "parts/sc_sctd.c"
#include "parts/sc_firw.c"
#include "parts/sc_firsb.c" /* sc_firs, sc_firb */
#include "parts/strPr.c"
#include "parts/prCh.c"
#include "parts/chk_encm.c"
#include "parts/cmd_upp.c"
#include "parts/chk_vwd.c"
#include "parts/prsCmd.c"
#include "parts/cmd_num.c"
#include "parts/lcp_upp.c"

#include "dat_u3b.c"
