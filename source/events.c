/* events.c -- deferred event queue drained by chk_actT(). */

#include "types.h"
#include "enums.h"
#include "ai.h"
#include "events.h"
#include "globals.h"

/* putEv -> parts/putEv.c (STX: 0x15fb4, after p_dobls). */

/* getEv -> parts/getEv.c (STX: 0x16002, right after putEv in the
   0x148fe object -- stx_u3.c includes it there). */
