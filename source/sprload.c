/*
 * sprload.c -- sprite pointer registration pass for the dog pipeline.
 */

#include "types.h"
#include "globals.h"
#include "sprglobs.h"
#include "sprload.h"

/* sp_mbuf: 14 KB shared mask buffer.  sp_regs writes a generated
   transparency mask here parallel to the sprite's image bytes in
   spr_file.  g_sedms[id] then references a slice
   here. */
unsigned char   sp_mbuf[14000];

/* sp_genma -> parts/sp_genma.c. */

/* sp_regs -> parts/sp_regs.c.  There is no driver loop function:
   main inlines the loop and calls sp_regs per sprite. */
