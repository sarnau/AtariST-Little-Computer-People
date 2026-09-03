/*
 * save.c -- HYBER save file I/O and the study-door save flow.
 * addr: crFile(), fr_read(), lcp_save(), lc_load(), lcp_std()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include <stdio.h>
#include "alerts.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "save.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"

/* fOpen -> parts/fOpen.c (STX: 0x730e). */

/* crFile -> parts/crFile.c (STX: 0x1488e, right after lcp_save). */

/* addr: fr_read() -- STX returns the Fread result. */
/* fr_read -> parts/fr_read.c (STX: 0x736c). */

/* fLoad does not exist in LCP_STX: al_loal is the only asset
   loader, and main inlines the .SCN path itself. */

/* lcp_save -> parts/lcp_save.c (STX: 0xdece object, 0x1481c). */

/* addr: lc_load() */
/* lc_load -> parts/lc_load.c (STX: 0x5ac8, in the 0x400c object
   ahead of gameLoop -- stx_u1.c includes it there). */

/* lcp_std -> parts/lcp_std.c (STX puts it immediately after
   a_opcuc in the 0xdece object). */
