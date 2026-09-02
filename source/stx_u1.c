/*
 * stx_u1.c -- STX unity translation unit for the 0x400c-0x73ce object.
 *
 * LCP_STX.PRG's game code is ~7 objects of 13-27 KB each, where the
 * port keeps 55 small ones (see CLAUDE.md, "Campaign #2").  Since
 * as68 shortens a call only when the callee is in the SAME assembly
 * unit, reproducing the STX call shapes requires reproducing its
 * object partition -- so the DEFAULT build compiles these sources as
 * one unit instead of individually.  FAITHFUL still compiles them
 * separately: LCP_ORG's partition is the port's own file list and is
 * already proven byte-identical.
 *
 * Membership and order come from stx_objmap.py's cluster report plus
 * the STX addresses of the byte-matched functions:
 *     dg_ipos 0x5aa0 < execEv 0x5fae < doAct 0x6038 < hs_posX 0x635e
 *     < daily_r 0x6c9e < pa_cloc 0x6cbe < er_nomem 0x73ce
 * Order only shifts addresses (verify_bytes wildcards displacements),
 * so it can be refined without affecting matching.
 *
 * alcyon_build.sh skips the constituents listed in
 * tools/stx_units.txt while building this file, and skips this file
 * under FAITHFUL.
 */

#ifndef FAITHFUL

/* cntSong is the FIRST function of this object (0x400c). */
#include "types.h"
#include "structs.h"
#include "enums.h"
#include "globals.h"
#include "init.h"
#include <osbind.h>
#include "parts/cntSong.c"  /* 0x400c */

#include "dog.c"
/* dg_mvAni (0x412c) is followed directly by walk.c's dg_wkPth
   (0x4586) -- the call between them is a bsr. */
#include "parts/dg_mvAni.c" /* 0x412c */
#include "parts/dg_wkPth.c" /* 0x4586 */
#include "ai.c"
/* chk_timA (0x6210) sits between doAct and hs_posX in this
   object, so airandom.c belongs here too. */
#include "airandom.c"
#include "actions.c"
/* walk.c straddles two STX objects: lcp_path (0x470a) and
   lcp_fstp (0x4fec) live here with getFlrY, while lcp_wkD and
   friends stay in the 0xdece object (stx_u2.c). */
#include "parts/wkCyc.c"
#include "parts/stairCyc.c"
#include "parts/setHTgt.c"
#include "parts/lcp_path.c"  /* 0x470a */
#include "parts/lcp_fstp.c"  /* 0x4fec */
/* assets.c straddles: the two asset loaders are in this object,
   right after getFlrY.  They need the trap bindings. */
#include <osbind.h>
#include "parts/ldObj.c"      /* 0x524a */
#include "parts/ldSpr.c"      /* 0x528a */
#include "parts/fr_reac.c"    /* 0x53b8 */
#include "movement.c"
#include "calendar.c"
#include "renderx.c"

/* fillTopR (0x686c) is in this object in LCP_STX, not the 0xdece
   one where render.c's other functions live. */
/* gfx_prim.c straddles too: cpyScr (0x64fa), stpScrB (0x6576) and
   sprites.c's sp_iniM (0x6612) are in this object, ahead of
   fillTopR. */
/* letload.c straddles: fl_ltpl is in this object, just ahead of
   cpyScr (its fr_reac call is a bsr). */
#include "parts/fl_ltpl.c"  /* 0x648c */
#include "parts/cpyScr.c"   /* 0x64fa */
#include "parts/stpScrB.c"  /* 0x6576 */
#include "parts/sp_iniM.c"  /* 0x6612 */
/* STX splits vdi_init: the opener (0x6680) and the attribute/clear
   half (0x66fe) it reaches with bsr.s. */
#include "parts/vdi_init.c" /* 0x6680 */
#include "parts/vdi_cls.c"  /* 0x66fe */
#include "parts/initBRev.c" /* 0x6804 */
#include "parts/rv_bld.c"   /* 0x680e -- bsr.s from initBRev */
#include "parts/fillTopR.c"

/* STX 0x69c6, just past getKey: the bare Random() wrapper. */
#include "parts/rnd.c"

/* save.c straddles too: fOpen and fr_read close this object. */
#include "parts/lc_load.c"   /* 0x5ac8 */

/* main.c straddles: gameLoop is in this object, between lc_load
   and chk_actT. */
#include "parts/gameLoop.c"   /* 0x5c76 */
#include "parts/chk_actT.c"   /* 0x5ce2 -- bsr.s from gameLoop */

#include "parts/fOpen.c"      /* 0x730e */
#include "parts/fr_read.c"    /* 0x736c */

/* Tail of the STX object: er_nomem (0x73ce).  alerts.c carries it
   for FAITHFUL instead. */
#include "parts/er_nomem.c"

#endif  /* !FAITHFUL */
