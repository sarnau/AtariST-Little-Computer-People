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
#include <osbind.h>       /* the sc_sdt* parts use Setscreen/Logbase */
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
#include "parts/sc_sdtb.c"  /* 0xe292 */
#include "parts/sc_sdtf.c"  /* 0xe310 */
#include "delivery.c"
#include "adoors.c"
#include "aleisure.c"
#include "parts/lcp_std.c"   /* 0xf534, right after a_opcuc */
#include "afood.c"      /* a_eatm 0x0ff14 */
#include "aidle.c"
#include "asimple.c"
#include "parts/hideLcp.c"
#include "parts/showLcp.c"
#include "parts/sp_ssco.c"   /* 0x1203a */
#include "parts/sp_ss02.c"   /* 0x12108 */
/* movement.c straddles: cWkday is in this object, just ahead of
   cl_drini (chk_timA reaches it with jsr, not bsr). */
#include "calendar.h"
#include "parts/daysInMo.c" /* 0x13796 */
#include "parts/cWkday.c"   /* 0x1332e */
#include "parts/cl_drini.c" /* 0x133b4 -- far from cl_redrH */
#include "abathrm.c"
#include "agames.c"   /* a_plaag 0x11860 */
#include "parts/a_opcfc.c" /* 0x11d9a, right after a_plaag */
#include "ahouse.c"
#include "parts/a_uset.c"  /* 0x101be */
#include "parts/a_clotd.c" /* 0x10556, right after a_uset */
#include "parts/a_drink.c" /* 0x121d6 */
#include "parts/updWtLv.c" /* 0x122fa, right after a_drink */
#include "parts/a_kitcc.c" /* 0x11354 */
#include "parts/a_playc.c" /* 0x12e86 */
#include "parts/tv_scrc.c" /* 0x13074 */
#include "tvanim.c"       /* tv_boul 0x130d6 */
#include "parts/tv_patl.c" /* 0x13204 */
#include "health.c"
/* wkFrDr reaches lcp_wkD (STX 0x147a0, inside this cluster) with a
   bsr, so the walk engine is part of this object too. */
/* The clock/line group: LCP_STX links these in this object.
   cl_drini 0x133b4, cl_redrH 0x137d4, drwLine 0x138d4,
   drwPixel 0x13930. */
#include "parts/cl_redrH.c" /* 0x137d4 */
#include "parts/cl_drwH.c"
#include "parts/drwLine.c"  /* 0x138d4 */
#include "parts/drwPixel.c" /* 0x13930 */
#include "parts/a_lists.c"  /* 0x1398c */
#include "parts/a_playp.c"  /* 0x13a62 */
#include "parts/rp_anim.c"  /* 0x13aec */
#include "parts/a_toggt.c"  /* 0x13bb2 */
#include "parts/tt_on.c"    /* 0x13bc8 */
#include "parts/tt_off.c"   /* 0x13c1e */
#include "parts/td_nois.c" /* 0x13c74 */
#include "parts/td_line.c"  /* 0x13c8a */
#include "parts/lt_sets.c" /* 0x1476c */
#include "parts/sfClick.c" /* 0x14786 */
#include "walk.c"

/* lcp_save (0x1481c) closes the STX object. */
#include "parts/lcp_save.c"
#include "parts/crFile.c"   /* 0x1488e */
#include "parts/er_write.c" /* 0x148e6 */

#endif  /* !FAITHFUL */
