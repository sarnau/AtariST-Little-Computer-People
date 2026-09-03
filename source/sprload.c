/*
 * sprload.c -- sprite pointer registration pass for the dog pipeline.
 * addr: spritedata_create_with_mask() @ Ghidra 0x15BDC,
 *       sp_reglp driver loop from main() at 0x1579c..0x15828.
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

/* sp_genma -> parts/sp_genma.c (STX: 0x408c, right after cntSong). */

/* sp_regs -> parts/sp_regs.c (STX: 0x5bdc, in the 0x400c object between lc_load and gameLoop). */

/* sp_reglp does not exist in LCP_STX: main inlines its loop and
   calls sp_regs (parts/sp_regs.c) per sprite. */
