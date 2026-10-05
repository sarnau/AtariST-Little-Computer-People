/*
 * sprload.c -- sprite pointer registration pass for the dog pipeline.
 */

#include "types.h"
#include "globals.h"
#include "sprglobs.h"
#include "sprload.h"

/* sp_mbuf: 14 KB shared mask buffer.  defineSprite writes a generated
   transparency mask here parallel to the sprite's image bytes in
   spr_file.  g_sedms[id] then references a slice
   here. */
unsigned char   sp_mbuf[14000];

/* makeMask -> parts/makeMask.c. */

/* defineSprite -> parts/defineSprite.c.  There is no driver loop function:
   main inlines the loop and calls defineSprite per sprite. */
