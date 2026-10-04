/* init.c -- boot-time init functions called from main(). */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "adoors.h"
#include "afood.h"
#include "agames.h"
#include "ahouse.h"
#include "aidle.h"
#include "aleisure.h"
#include "assets.h"
#include "calendar.h"
#include "delivery.h"
#include "dog.h"
#include "events.h"
#include "gfx_prim.h"
#include "globals.h"
#include "init.h"
#include "keyboard.h"
#include "midi_seq.h"
#include "movement.h"
#include "parser.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tables.h"
#include "walk.h"


/* lcp_crnd -> parts/lcp_crnd.c. */

/* cl_drini -> parts/cl_drini.c. */

/* st_titl -> parts/st_titl.c. */

/* mq_intim -> parts/mq_intim.c (included by midi_seq.c). */

/* cntSong -> parts/cntSong.c. */

/* cl_redrH, cl_drwH and drwLine belong to stx_u2's object, so stx_u2.c
   includes parts/cl_redrH.c, parts/cl_drwH.c and parts/drwLine.c. */

/* initBRev -> parts/initBRev.c, with the builder it calls in
   parts/rv_bld.c right behind it; stx_u1.c includes both. */

/* cs_mvIn -> parts/cs_mvIn.c (included by stx_u2.c). */
