/*
 * keyboard.c -- keyboard polling + Ctrl-key event dispatch.
 * addr: getKey(), deal_kc()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "ai.h"
#include "events.h"
#include "globals.h"
#include "keyboard.h"
#include "render.h"
#include "renderx.h"
#include "sound.h"


/* getKey -> parts/getKey.c (STX: 0x68ee, in the 0x400c object
   just before rnd -- stx_u1.c includes it there). */
#ifdef FAITHFUL
#include "parts/getKey.c"
#endif

/* deal_kc -> parts/deal_kc.c (STX: 0x15d72, stx_u3 object). */
#ifdef FAITHFUL
#include "parts/deal_kc.c"
#endif
