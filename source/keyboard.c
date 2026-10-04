/*
 * keyboard.c -- keyboard polling + Ctrl-key event dispatch.
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


/* getKey lives in parts/getKey.c, included by stx_u1.c just before
   rnd. */

/* deal_kc lives in parts/deal_kc.c, included by stx_u3.c. */
