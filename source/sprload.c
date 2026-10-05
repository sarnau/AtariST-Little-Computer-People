/*
 * sprload.c -- sprite pointer registration pass for the dog pipeline.
 */

#include "types.h"
#include "globals.h"
#include "sprglobs.h"
#include "sprload.h"

/* genMaskBuf: 14 KB shared mask buffer.  defineSprite writes a generated
   transparency mask here parallel to the sprite's image bytes in
   sprFileBuf.  spriteMask[id] then references a slice
   here. */
unsigned char   genMaskBuf[14000];

/* makeMask -> parts/makeMask.c. */

/* defineSprite -> parts/defineSprite.c.  There is no driver loop function:
   main inlines the loop and calls defineSprite per sprite. */
