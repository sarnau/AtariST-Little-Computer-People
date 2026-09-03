/*
 * stx_u2.c -- STX unity unit for the 0x0de36-0x148fe object (27 KB,
 * the largest in LCP_STX).  See stx_u1.c for the rationale and
 * CLAUDE.md for the cluster evidence.
 *
 * The include list below IS the object's function order, recovered by
 * pairing every LCP_STX prologue span in 0xde36..0x148fe against the
 * port's byte-identical counterpart.  LCP_STX did not group this
 * object by source file -- aleisure's nine functions alone are spread
 * from 0xe338 to 0x12ca0 -- so the port has no action .c files left at
 * all: every body lives in parts/ and this list is the order.
 *
 * alcyon_build.sh skips the constituents listed in
 * tools/stx_units.txt while building this file.
 */


/* Headers first: they emit no code, so the object layout is
   unaffected, but the parts/ bodies below need them in scope. */
#include "types.h"
#include <osbind.h>       /* the sc_sdt* parts use Setscreen/Logbase */
#include <stdio.h>        /* sprintf, for the letter writer */
#include <vdibind.h>
#include "structs.h"
#include "enums.h"
#include "obdefs1.h"
#include "globals.h"
#include "abathrm.h"
#include "actions.h"
#include "adoors.h"
#include "afood.h"
#include "agames.h"
#include "ahouse.h"
#include "aidle.h"
#include "aleisure.h"
#include "alerts.h"
#include "aletter.h"
#include "asimple.h"
#include "calendar.h"
#include "clock.h"
#include "delivery.h"
#include "events.h"
#include "games.h"
#include "gfx_prim.h"
#include "health.h"
#include "keyboard.h"
#include "letload.h"
#include "midi_seq.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "save.h"
#include "sim.h"
#include "sound.h"
#include "sprender.h"
#include "sprglobs.h"
#include "sprhead.h"
#include "sprites.h"
#include "tables.h"
#include "tick.h"
#include "tvanim.h"
#include "vdiown.h"
#include "walk.h"

#include "parts/moffmon.c"     /* moff 0xde36, mon 0xde5c */
#include "parts/lcp_lgt.c"     /* 0xde80 */
#include "parts/lcp_rgt.c"     /* 0xdf66 */
#include "parts/sp_sprs.c"     /* 0xe0b2 */
#include "render.c"            /* od_draw 0xe160, sc_drfc 0xe1f4 */
#include "parts/sc_sdtb.c"     /* 0xe292 */
#include "parts/sc_sdtf.c"     /* 0xe310 */
#include "parts/a_chefd.c"     /* 0xe338 */
#include "parts/hideLcp.c"     /* 0xe4ae */
#include "parts/showLcp.c"     /* 0xe4de */
#include "parts/cs_mvIn.c"     /* 0xe500 */
#include "parts/a_wakfa.c"     /* 0xe8c8 */
#include "parts/a_cleau.c"     /* 0xe912 */
#include "parts/a_clocd.c"     /* 0xeb54 */
#include "parts/a_gesff.c"     /* 0xebf8 */
#include "parts/a_opecf.c"     /* 0xec22 */
#include "parts/a_opecd.c"     /* 0xed50 */
#include "parts/a_opecc.c"     /* 0xee4e */
#include "parts/a_opcbc.c"     /* 0xef54 */
#include "parts/a_opcuc.c"     /* 0xf358 */
#include "parts/lcp_std.c"     /* 0xf534 */
#include "parts/a_hello.c"     /* 0xf7ce */
/* STX order here is tvc, spe, hnd, grt -- the sound ids in the
   wrappers at 0xf91e (3,7) and 0xf952 (2,9) settle it, and ev_ansPh's
   two bsr displacements agree. */
#include "parts/p_sftvc.c"     /* 0xf904 */
#include "parts/p_sfspe.c"     /* 0xf91e */
#include "parts/p_sfhnd.c"     /* 0xf938 */
#include "parts/p_sfgrt.c"     /* 0xf952 */
#include "parts/a_plawr.c"     /* 0xf96c */
#include "parts/a_feedd.c"     /* 0xfca0 */
#include "parts/wkFrDr.c"      /* 0xfef0 */
#include "parts/a_eatm.c"      /* 0xff14 */
#include "parts/a_opcfd.c"     /* 0x100cc */
#include "parts/a_uset.c"      /* 0x101be */
#include "parts/a_clotd.c"     /* 0x10556 */
#include "parts/a_takes.c"     /* 0x105fa */
#include "parts/a_petd.c"      /* 0x107ac */
#include "parts/a_calld.c"     /* 0x1081e */
#include "parts/a_watat.c"     /* 0x1087c */
#include "parts/a_tidyh.c"     /* 0x10958 */
#include "parts/ev_ansPh.c"    /* 0x109c2 */
#include "parts/a_socwd.c"     /* 0x10b98 */
#include "parts/er_dogf.c"     /* 0x10cf4 */
#include "parts/er_recd.c"     /* 0x10d0e */
#include "parts/a_lighf.c"     /* 0x10e4c */
#include "parts/er_food.c"     /* 0x1107c */
#include "parts/er_bood.c"     /* 0x1123e */
#include "parts/a_kitcc.c"     /* 0x11354 */
#include "parts/a_brust.c"     /* 0x11736 */
#include "agames.c"            /* a_plaag 0x11860 */
#include "parts/a_opcfc.c"     /* 0x11d9a */
#include "parts/a_peeka.c"     /* 0x11e34 */
#include "parts/a_getd.c"      /* 0x11e9c */
#include "parts/a_nodh.c"      /* 0x11f82 */
#include "parts/sp_ssco.c"     /* 0x1203a */
#include "parts/sp_ss02.c"     /* 0x12108 */
#include "parts/a_drink.c"     /* 0x121d6 */
#include "parts/updWtLv.c"     /* 0x122fa */
#include "parts/a_driwa.c"     /* 0x124da */
#include "parts/a_pacen.c"     /* 0x12636 */
#include "parts/a_wandi.c"     /* 0x126ae */
#include "parts/a_sleep.c"     /* 0x1272e */
#include "parts/a_dance.c"     /* 0x12854 */
#include "parts/a_yawas.c"     /* 0x12912 */
#include "parts/a_washh.c"     /* 0x1298a */
#include "parts/a_gioob.c"     /* 0x12adc */
#include "parts/li_loor.c"     /* 0x12c08 */
#include "parts/li_lool.c"     /* 0x12c54 */
#include "parts/a_sitae.c"     /* 0x12ca0 */
#include "parts/a_readn.c"     /* 0x12d8e */
#include "parts/a_playc.c"     /* 0x12e86 */
#include "parts/tv_scrc.c"     /* 0x13074 */
#include "tvanim.c"            /* tv_boul 0x130d6 */
#include "parts/tv_patl.c"     /* 0x13204 */
#include "parts/cWkday.c"      /* 0x1332e */
#include "parts/cl_drini.c"    /* 0x133b4 */
#include "sim.c"               /* gameSim1 0x133da */
#include "health.c"            /* lcp_sick 0x13630, lcp_rcov 0x1366a, lcp_upal 0x13692 */
#include "parts/a_wakum.c"     /* 0x136c6 */
#include "parts/a_gotbn.c"     /* 0x13748 */
#include "parts/daysInMo.c"    /* 0x13796 */
#include "parts/cl_redrH.c"    /* 0x137d4 */
#include "parts/cl_drwH.c"     /* 0x13826 */
#include "parts/drwLine.c"     /* 0x138d4 */
#include "parts/drwPixel.c"    /* 0x13930 */
#include "parts/a_lists.c"     /* 0x1398c */
#include "parts/a_playp.c"     /* 0x13a62 */
#include "parts/rp_anim.c"     /* 0x13aec */
#include "parts/a_toggt.c"     /* 0x13bb2 */
#include "parts/tt_on.c"       /* 0x13bc8 */
#include "parts/tt_off.c"      /* 0x13c1e */
#include "parts/td_nois.c"     /* 0x13c74 */
#include "parts/td_line.c"     /* 0x13c8a */
#include "parts/a_writl.c"     /* 0x13cd6 */
#include "parts/lt_tysa.c"     /* 0x1434a */
#include "parts/lt_tyca.c"     /* 0x1445c */
#include "parts/lt_sets.c"     /* 0x1476c */
#include "parts/sfClick.c"     /* 0x14786 */
#include "walk.c"              /* lcp_wkD 0x147a0 */
#include "parts/lcp_save.c"    /* 0x1481c */
#include "parts/crFile.c"      /* 0x1488e */
#include "parts/er_write.c"    /* 0x148e6 */

