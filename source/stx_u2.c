/*
 * stx_u2.c -- STX unity unit for the 0x0dece-0x1481c object (27 KB,
 * the largest in LCP_STX).  See stx_u1.c for the rationale and
 * CLAUDE.md for the cluster evidence.
 *
 * Membership here is the subset of that cluster whose port files sit
 * wholly inside it; the four straddlers (games.c, gfx_prim.c, init.c,
 * sprites.c) contribute only part of their functions and join once
 * they are split.  Order follows the byte-matched members' STX
 * addresses:
 *     od_draw 0xe160 < wkFrDr 0xe338 < a_clocd 0xeb54
 *     < a_opcuc 0xf358 < a_kitcc 0x11354 < a_peeka 0x11e34
 *     < a_nodh 0x11f82
 *     < a_driwa 0x124da < li_loor 0x12c54 < lcp_sick 0x13630
 *     < lcp_wkD 0x147a0
 */

#ifndef FAITHFUL

/* Headers first: they emit no code, so the object layout is
   unaffected, but the parts/ bodies below need them in scope. */
#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "sprender.h"
#include "sprglobs.h"
#include "sprhead.h"
#include "sprites.h"
#include "tables.h"
#include "tick.h"

#include "parts/lcp_lgt.c"   /* 0xde80 */
#include "parts/lcp_rgt.c"   /* 0xdf66 */
#include "parts/sp_sprs.c"   /* 0xe0b2, before od_draw */
#include "render.c"
#include "delivery.c"
#include "adoors.c"
#include "aleisure.c"
#include "parts/lcp_std.c"   /* 0xf534, right after a_opcuc */
#include "afood.c"      /* a_kitcc 0x11354 */
#include "aidle.c"
#include "asimple.c"
#include "parts/hideLcp.c"
#include "parts/showLcp.c"
#include "parts/sp_ssco.c"   /* 0x1203a */
#include "abathrm.c"
#include "ahouse.c"
#include "health.c"
/* wkFrDr reaches lcp_wkD (STX 0x147a0, inside this cluster) with a
   bsr, so the walk engine is part of this object too. */
#include "walk.c"

#endif  /* !FAITHFUL */
