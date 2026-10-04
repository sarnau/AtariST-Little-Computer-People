/*
 * stx_u1.c -- unity translation unit for the first of the original's
 * game-code objects.
 *
 * The original's game code is ~7 large objects, where the port keeps
 * many small source files.  as68 shortens a call only when the callee
 * is in the SAME assembly unit, so reproducing the original's call
 * shapes requires reproducing its object partition -- the default
 * build therefore compiles these sources as one unit.
 *
 * The ORDER of the #include lines below is the object's function (and
 * data) order and must not change.
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
#ifdef HOST
#include "hostgem.h"
#else
#include <vdibind.h>
#endif
#include <osbind.h>
#include "globals.h"
#include "abathrm.h"
#include "alerts.h"
#include "assets.h"
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
#include "render.h"
#include "renderx.h"
#include "save.h"
#include "sprglobs.h"
#include "sprites.h"
#include "walk.h"

#include "dat_u1.c"


#include "parts/cntSong.c"
#include "parts/sp_genma.c"
/* dg_mvAni is followed directly by walk.c's dg_wkPth. */
#include "parts/dg_mvAni.c"
#include "parts/dg_wkPth.c"
/* walk.c straddles two objects: lcp_path and lcp_fstp live here
   with getFlrY, while lcp_wkD and friends are in stx_u2.c. */
#include "parts/lcp_path.c"
#include "parts/lcp_fstp.c"
#include "parts/lcp_flwp.c"
#include "parts/getFlrY.c"
/* assets.c straddles: the two asset loaders are in this object,
   right after getFlrY.  They need the trap bindings. */
#include "parts/ldObj.c"
#include "parts/ldSpr.c"
#include "parts/scn_dec.c"
#include "parts/fr_reac.c"
#include "main.h"
#include "stubs.h"
#include "sprload.h"
#include "sprender.h"
#include "tables.h"
#include "assets.h"
#include "tick_tables.h"
#include "dat_u1d.c"
#include "parts/main.c"
#include "dog.c"
/* save.c straddles too: lc_load and sp_regs sit between dg_ipos and
   gameLoop. */
#include "parts/lc_load.c"
#include "parts/sp_regs.c"
/* main.c straddles: gameLoop is in this object, between sp_regs
   and execEv. */
#include "parts/gameLoop.c"
#include "parts/chk_actT.c"
#include "ai.c"
#include "actions.c"
/* execEv's and doAct's switch jump tables land in the data segment
   right here, so the globals that follow them come after this point,
   not with the rest at the top. */
#include "dat_u1b.c"
/* chk_timA sits between doAct and hs_posXY. */
#include "airandom.c"
#include "movement.c"
#include "parts/vroCpyD.c"
/* letload.c straddles: fl_ltpl is in this object, just ahead of
   cpyScr.  gfx_prim.c straddles too: cpyScr, stpScrB and sprites.c's
   sp_iniM are in this object. */
#include "parts/al_loal.c"
#include "parts/fl_ltpl.c"
#include "parts/cpyScr.c"
#include "parts/stpScrB.c"
#include "parts/sp_iniM.c"
/* vdi_init is split in two: the opener, and the attribute/clear half
   it calls, which must follow it directly. */
#include "parts/vdi_init.c"
#include "parts/vdi_cls.c"
#include "parts/aes_init.c"
#include "parts/initBRev.c"
#include "parts/rv_bld.c"
/* fillTopR is in this object, not stx_u2's where render.c's other
   functions live. */
#include "parts/fillTopR.c"
#include "parts/getKey.c"
/* getKey's jump table lands in the data segment here, so the last
   globals of this unit are declared behind it. */
#include "dat_u1c.c"
/* The bare Random() wrapper, just past getKey. */
#include "parts/rnd.c"
#include "parts/lcp_crnd.c"
#include "calendar.c"
#include "renderx.c"
/* st_titl is a real interactive title screen. */
#include "parts/st_titl.c"
#include "parts/stEnter.c"
#include "parts/erChr.c"
/* save.c's file helpers come near the end of this object. */
#include "parts/fOpen.c"
#include "parts/fr_read.c"
/* er_nomem closes the object. */
#include "parts/er_nomem.c"

