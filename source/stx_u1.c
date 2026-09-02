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

#include "dog.c"
#include "ai.c"
#include "actions.c"
/* walk.c straddles two STX objects: lcp_path (0x470a) and
   lcp_fstp (0x4fec) live here with getFlrY, while lcp_wkD and
   friends stay in the 0xdece object (stx_u2.c). */
#include "parts/wkCyc.c"
#include "parts/stairCyc.c"
#include "parts/setHTgt.c"
#include "parts/lcp_path.c"  /* 0x470a */
#include "parts/lcp_fstp.c"  /* 0x4fec */
#include "movement.c"
#include "calendar.c"
#include "renderx.c"

/* fillTopR (0x686c) is in this object in LCP_STX, not the 0xdece
   one where render.c's other functions live. */
/* gfx_prim.c straddles too: cpyScr (0x64fa), stpScrB (0x6576) and
   sprites.c's sp_iniM (0x6612) are in this object, ahead of
   fillTopR. */
#include "parts/cpyScr.c"   /* 0x64fa */
#include "parts/stpScrB.c"  /* 0x6576 */
#include "parts/sp_iniM.c"  /* 0x6612 */
#include "parts/fillTopR.c"

/* Tail of the STX object: er_nomem (0x73ce).  alerts.c carries it
   for FAITHFUL instead. */
#include "parts/er_nomem.c"

#endif  /* !FAITHFUL */
