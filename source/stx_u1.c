/*
 * stx_u1.c -- STX unity translation unit for the 0x400c-0x73ce object.
 *
 * LCP_STX.PRG's game code is ~7 objects of 13-27 KB each, where the
 * port keeps 55 small ones (see CLAUDE.md, "Campaign #2").  Since
 * as68 shortens a call only when the callee is in the SAME assembly
 * unit, reproducing the STX call shapes requires reproducing its
 * object partition -- so the DEFAULT build compiles these sources as
 * one unit instead of individually.
 *
 * Membership and order come from stx_objmap.py's cluster report plus
 * the STX addresses of the byte-matched functions:
 *     dg_ipos 0x5aa0 < execEv 0x5fae < doAct 0x6038 < hs_posX 0x635e
 *     < daily_r 0x6c9e < pa_cloc 0x6cbe < er_nomem 0x73ce
 * Order only shifts addresses (verify_bytes wildcards displacements),
 * so it can be refined without affecting matching.
 *
 * alcyon_build.sh skips the constituents listed in
 * tools/stx_units.txt while building this file.
 */


/* Headers first: they emit no code, so the object layout is
   unaffected, but the parts/ bodies below need them in scope. */
#include "types.h"
#include "structs.h"
#include "enums.h"
#include "obdefs1.h"
#include <vdibind.h>
#include <osbind.h>
#include "globals.h"
#include "abathrm.h"
#include "actions.h"
#include "afood.h"
#include "agames.h"
#include "ahouse.h"
#include "ai.h"
#include "aidle.h"
#include "airandom.h"
#include "aleisure.h"
#include "aletter.h"
#include "asimple.h"
#include "calendar.h"
#include "delivery.h"
#include "dog.h"
#include "events.h"
#include "gfx_prim.h"
#include "init.h"
#include "movement.h"
#include "parser.h"
#include "random.h"
#include "renderx.h"
#include "save.h"
#include "sprglobs.h"
#include "sprites.h"
#include "walk.h"

#include "dat_u1.c"


#include "parts/cntSong.c"  /* 0x400c */
#include "parts/sp_genma.c" /* 0x408c */
/* dg_mvAni (0x412c) is followed directly by walk.c's dg_wkPth
   (0x4586) -- the call between them is a bsr. */
#include "parts/dg_mvAni.c" /* 0x412c */
#include "parts/dg_wkPth.c" /* 0x4586 */
/* walk.c straddles two STX objects: lcp_path (0x470a) and
   lcp_fstp (0x4fec) live here with getFlrY, while lcp_wkD and
   friends stay in the 0xdece object (stx_u2.c). */
#include "parts/lcp_path.c"  /* 0x470a */
#include "parts/lcp_fstp.c"  /* 0x4fec */
#include "parts/lcp_flwp.c"  /* 0x50bc */
#include "parts/getFlrY.c"   /* 0x5224, reached by bsr from lcp_flwp */
/* assets.c straddles: the two asset loaders are in this object,
   right after getFlrY.  They need the trap bindings. */
#include "parts/ldObj.c"     /* 0x524a */
#include "parts/ldSpr.c"     /* 0x528a */
#include "parts/scn_dec.c"   /* 0x52ca */
#include "parts/fr_reac.c"   /* 0x53b8 */
#include "main.h"
#include "stubs.h"
#include "sprload.h"
#include "sprender.h"
#include "tables.h"
#include "assets.h"
#include "tick_tables.h"
#include "parts/main.c"      /* 0x5546 */
#include "dog.c"             /* dg_ipos 0x5aa0 */
/* save.c straddles too: lc_load and sp_regs sit between dg_ipos and
   gameLoop. */
#include "parts/lc_load.c"   /* 0x5ac8 */
#include "parts/sp_regs.c"   /* 0x5bdc */
/* main.c straddles: gameLoop is in this object, between sp_regs
   and execEv. */
#include "parts/gameLoop.c"  /* 0x5c76 */
#include "parts/chk_actT.c"  /* 0x5ce2 -- bsr.s from gameLoop */
#include "ai.c"              /* execEv 0x5fae */
#include "actions.c"         /* doAct 0x6038 */
/* chk_timA (0x6210) sits between doAct and hs_posXY. */
#include "airandom.c"        /* chk_timA 0x6210 */
#include "movement.c"        /* hs_posXY 0x635e */
#include "parts/vroCpyD.c"   /* 0x63cc */
/* letload.c straddles: fl_ltpl is in this object, just ahead of
   cpyScr (its fr_reac call is a bsr).  gfx_prim.c straddles too:
   cpyScr (0x64fa), stpScrB (0x6576) and sprites.c's sp_iniM
   (0x6612) are in this object. */
#include "parts/al_loal.c"   /* 0x6428 */
#include "parts/fl_ltpl.c"   /* 0x648c */
#include "parts/cpyScr.c"    /* 0x64fa */
#include "parts/stpScrB.c"   /* 0x6576 */
#include "parts/sp_iniM.c"   /* 0x6612 */
/* STX splits vdi_init: the opener (0x6680) and the attribute/clear
   half (0x66fe) it reaches with bsr.s. */
#include "parts/vdi_init.c"  /* 0x6680 */
#include "parts/vdi_cls.c"   /* 0x66fe */
#include "parts/aes_init.c"  /* 0x67aa */
#include "parts/initBRev.c"  /* 0x6804 */
#include "parts/rv_bld.c"    /* 0x680e -- bsr.s from initBRev */
/* fillTopR (0x686c) is in this object in LCP_STX, not the 0xdece
   one where render.c's other functions live. */
#include "parts/fillTopR.c"  /* 0x686c */
#include "parts/getKey.c"    /* 0x68ee */
/* STX 0x69c6, just past getKey: the bare Random() wrapper. */
#include "parts/rnd.c"       /* 0x69c6 */
#include "parts/lcp_crnd.c"  /* 0x69d8 */
#include "calendar.c"        /* daily_rs 0x6c9e */
#include "renderx.c"         /* pa_cloc 0x6cbe, pa_skic 0x6d1e */
/* st_titl (0x6d7e) is a real interactive title screen in STX. */
#include "parts/st_titl.c"   /* 0x6d7e */
#include "parts/stEnter.c"   /* 0x718e */
#include "parts/erChr.c"     /* 0x72e6 */
/* save.c's file helpers close this object. */
#include "parts/fOpen.c"     /* 0x730e */
#include "parts/fr_read.c"   /* 0x736c */
/* Tail of the STX object: er_nomem (0x73ce). */
#include "parts/er_nomem.c"  /* 0x73ce */

