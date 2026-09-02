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

#include "render.c"
#include "delivery.c"
#include "adoors.c"
#include "aleisure.c"
#include "afood.c"      /* a_kitcc 0x11354 */
#include "aidle.c"
#include "asimple.c"
#include "abathrm.c"
#include "ahouse.c"
#include "health.c"
/* wkFrDr reaches lcp_wkD (STX 0x147a0, inside this cluster) with a
   bsr, so the walk engine is part of this object too. */
#include "walk.c"

#endif  /* !FAITHFUL */
