/*
 * stx_u3.c -- STX unity unit for the 0x14824-0x172e8 sprite object.
 *
 * Evidence: er_write 0x14824, sp_spud 0x148fe, sp_flih 0x14a76 and
 * sp_updb 0x16244 all fall in this range, and er_write's cluster
 * ends 24 bytes before sp_spud's begins -- one object.  See
 * stx_u1.c for the mechanism and CLAUDE.md for the cluster report.
 */

#ifndef FAITHFUL

/* prCh needs obdefs.h (MD_TRANS/MD_REPLACE); renderx.c pulls it
   in for the FAITHFUL build. */
#include <obdefs.h>

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
#include "parts/sp_drin.c"
#include "parts/sp_updb.c"  /* 0x16244 */
#include "parts/sp_lchu.c"  /* 0x16632 */
#include "parts/strPr.c"   /* 0x16ea8 */
#include "parts/prCh.c"    /* 0x16ede */
#include "parts/chk_encm.c"/* 0x16f9a */
#include "parts/prsCmd.c"  /* 0x1721c */

#endif  /* !FAITHFUL */
