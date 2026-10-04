/*
 * save.c -- HYBER save file I/O and the study-door save flow.
 * The function bodies live in parts/ and are included by the unity
 * units at their places in the binary.
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

/* fOpen -> parts/fOpen.c, included by stx_u1.c. */

/* crFile -> parts/crFile.c, included by stx_u2.c right after
   lcp_save. */

/* fr_read -> parts/fr_read.c, included by stx_u1.c; it returns the
   Fread result. */

/* There is no separate file loader: al_loal is the only asset
   loader, and main handles the .SCN file itself. */

/* lcp_save -> parts/lcp_save.c, included by stx_u2.c. */

/* lc_load -> parts/lc_load.c, included by stx_u1.c ahead of
   gameLoop. */

/* lcp_std -> parts/lcp_std.c, included by stx_u2.c immediately after
   a_opcuc. */
