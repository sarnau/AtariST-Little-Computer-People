/*
 * stx_u3.c -- STX unity unit for the 0x14824-0x172e8 sprite object.
 *
 * Evidence: er_write 0x14824, sp_spud 0x148fe, sp_flih 0x14a76 and
 * sp_updb 0x16244 all fall in this range, and er_write's cluster
 * ends 24 bytes before sp_spud's begins -- one object.  See
 * stx_u1.c for the mechanism and CLAUDE.md for the cluster report.
 */


/* prCh needs obdefs.h (MD_TRANS/MD_REPLACE). */
#include "obdefs1.h"

/* deal_kc/putEv need the globals and prototypes their own files
   pull in. */
#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "ai.h"
#include "events.h"
#include "keyboard.h"
#include "sound.h"
#include "render.h"
#include "renderx.h"
#include "parser.h"
#include "vocab.h"

#include "alerts.c"
#include "sprites.c"
/* renderf.c straddles: sc_ren8 is in this object (0x15138), ahead
   of lcp_hwt. */
#include "parts/sc_ren8.c"  /* 0x15138 */
/* STX: lcp_hwt (0x1568a) immediately precedes gameTick (0x156a6). */
#include "parts/lcp_hwt.c"
#include "tick.c"
/* LCP_STX puts these after gameTick: sp_drin 0x1623c,
   sp_updb 0x16244, sp_lchu 0x16632. */
/* LCP_STX links these after gameTick: deal_kc 0x15d72,
   p_dobls 0x15f9a, putEv 0x15fb4. */
#include "parts/deal_kc.c"  /* 0x15d72 */
#include "parts/p_dobls.c"  /* 0x15f9a */
#include "parts/putEv.c"    /* 0x15fb4 */
#include "parts/getEv.c"    /* 0x16002 */
#include "parts/sp_draw.c"  /* 0x1605c */
#include "parts/sp_drin.c"
#include "parts/sp_updb.c"  /* 0x16244 */
#include "parts/sp_lcha.c"  /* 0x16368 */
#include "parts/sp_lchu.c"  /* 0x1664c */
/* sprites.c straddles within this object: these four sit past
   sp_lchu, not with sp_upds/sp_imfs at the front. */
#include "parts/sp_lbal.c"  /* 0x167b0 */
#include "parts/sp_lbbd.c"  /* 0x1682e */
#include "parts/sp_lbhd.c"  /* 0x169b4 */
#include "parts/sp_lcpf.c"  /* 0x16ae4 */
/* renderx.c and gfx_prim.c straddle: sc_sctd and sc_firw sit in
   this object, and sc_firw must follow sc_sctd (bsr.s). */
#include "parts/sc_sctd.c"  /* 0x16d5a */
#include "parts/sc_firw.c"  /* 0x16dcc */
#include "parts/sc_firsb.c" /* sc_firs 0x16e22, sc_firb 0x16e76 */
#include "parts/strPr.c"   /* 0x16ea8 */
#include "parts/prCh.c"    /* 0x16ede */
#include "parts/chk_encm.c"/* 0x16f9a */
#include "parts/cmd_upp.c" /* 0x1711c */
#include "parts/chk_vwd.c" /* 0x171ae */
#include "parts/prsCmd.c"  /* 0x1721c */
#include "parts/cmd_num.c" /* 0x17278 */
#include "parts/lcp_upp.c" /* 0x172e8 */

