/*
 * stx_u2.c -- unity unit for the largest game object: rendering,
 * sprites, the action bodies, the TV, health and the letter writer.
 * See stx_u1.c for the rationale.
 *
 * The include list below IS the object's function order and must not
 * change.  The original did not group this object by source file --
 * the leisure actions alone are spread across most of it -- so the
 * port has no action .c files left at all: every body lives in parts/
 * and this list is the order.
 *
 * alcyon_build.sh skips the constituents listed in
 * tools/stx_units.txt while building this file.
 */


/* Headers first: they emit no code, so the object layout is
   unaffected, but the parts/ bodies below need them in scope. */
#include "types.h"
#include <osbind.h>       /* the sc_sdt* parts use Setscreen/Logbase */
#include <stdio.h>        /* sprintf, for the letter writer */
#ifdef HOST
#include "hostgem.h"
#else
#include <vdibind.h>
#endif
#include "structs.h"
#include "enums.h"
#include "obdefs1.h"
#include "globals.h"
#include "protos.h"
#include "rnd.h"
#include "calendar.h"
#include "events.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tables.h"
#include "vdiown.h"

#include "dat_u2.c"


#include "parts/moffmon.c"     /* moff, mon */
#include "parts/lcp_lgt.c"
#include "parts/lcp_rgt.c"
#include "parts/sp_sprs.c"
#include "render.c"            /* od_draw, sc_drfc */
#include "parts/sc_sdtb.c"
#include "parts/sc_sdtf.c"
#include "parts/a_chefd.c"
#include "parts/hideLcp.c"
#include "parts/showLcp.c"
#include "parts/cs_mvIn.c"
#include "parts/a_wakfa.c"
#include "parts/a_cleau.c"
#include "parts/a_clocd.c"
#include "parts/a_gesff.c"
#include "parts/a_opecf.c"
#include "parts/a_opecd.c"
#include "parts/a_opecc.c"
#include "parts/a_opcbc.c"
#include "parts/a_opcuc.c"
#include "parts/lcp_std.c"
#include "parts/a_hello.c"
/* Order here is tvc, spe, hnd, grt -- the sound ids in the wrappers
   settle it. */
#include "parts/p_sftvc.c"
#include "parts/p_sfspe.c"
#include "parts/p_sfhnd.c"
#include "parts/p_sfgrt.c"
#include "parts/a_plawr.c"
#include "parts/a_feedd.c"
#include "parts/wkFrDr.c"
#include "parts/a_eatm.c"
#include "parts/a_opcfd.c"
#include "parts/a_uset.c"
#include "parts/a_clotd.c"
#include "parts/a_takes.c"
#include "parts/a_petd.c"
#include "parts/a_calld.c"
#include "parts/a_watat.c"
#include "parts/a_tidyh.c"
#include "parts/ev_ansPh.c"
#include "parts/a_socwd.c"
#include "parts/er_dogf.c"
#include "parts/er_recd.c"
#include "parts/a_lighf.c"
#include "parts/er_food.c"
#include "parts/er_bood.c"
#include "parts/a_kitcc.c"
#include "parts/a_brust.c"
#include "agames.c"            /* a_plaag */
#include "parts/a_opcfc.c"
#include "parts/a_peeka.c"
#include "parts/a_getd.c"
#include "parts/a_nodh.c"
#include "parts/sp_ssco.c"
#include "parts/sp_ss02.c"
#include "parts/a_drink.c"
#include "parts/updWtLv.c"
#include "parts/a_driwa.c"
#include "parts/a_pacen.c"
#include "parts/a_wandi.c"
#include "parts/a_sleep.c"
#include "parts/a_dance.c"
#include "parts/a_yawas.c"
#include "parts/a_washh.c"
#include "parts/a_gioob.c"
#include "parts/li_loor.c"
#include "parts/li_lool.c"
#include "parts/a_sitae.c"
#include "parts/a_readn.c"
#include "parts/a_playc.c"
#include "parts/tv_scrc.c"
#include "tvanim.c"            /* tv_boul */
#include "parts/tv_patl.c"
#include "parts/cWkday.c"
#include "parts/cl_drini.c"
#include "sim.c"               /* gameSim1 */
#include "health.c"            /* lcp_sick, lcp_rcov, lcp_upal */
#include "parts/a_wakum.c"
#include "parts/a_gotbn.c"
#include "parts/daysInMo.c"
#include "parts/cl_redrH.c"
#include "parts/cl_drwH.c"
#include "parts/drwLine.c"
#include "parts/drwPixel.c"
#include "parts/a_lists.c"
#include "parts/a_playp.c"
#include "parts/rp_anim.c"
#include "parts/a_toggt.c"
#include "parts/tt_on.c"
#include "parts/tt_off.c"
#include "parts/td_nois.c"
#include "parts/td_line.c"
#include "dat_u2b.c"
#include "parts/a_writl.c"
#include "parts/lt_tysa.c"
#include "parts/lt_tyca.c"
#include "parts/lt_sets.c"
#include "parts/sfClick.c"
#include "walk.c"              /* lcp_wkD */
#include "parts/lcp_save.c"
#include "parts/crFile.c"
#include "parts/er_write.c"

