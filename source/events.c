/* events.c -- deferred event queue drained by chk_actT(). */

#include "types.h"
#include "enums.h"
#include "ai.h"
#include "events.h"
#include "globals.h"

/* putEv and getEv live in parts/putEv.c and parts/getEv.c; stx_u3.c
   includes them, putEv right after p_dobls and getEv right after
   putEv. */
