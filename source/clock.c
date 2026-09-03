/* clock.c -- analog clock hand renderer.  Clock face at (278, 85). */

#include "types.h"
#include "enums.h"
#include "clock.h"
#include "gfx_prim.h"
#include "globals.h"

/* Y offset uses index + 3 (quarter-turn phase shift) so the same
   15-entry table serves both axes.
   addr: cl_drwH() */
