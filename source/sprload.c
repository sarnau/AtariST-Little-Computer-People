/*
 * sprload.c -- sprite pointer registration pass for the dog pipeline.
 * addr: spritedata_create_with_mask() @ Ghidra 0x15BDC,
 *       sp_reglp driver loop from main() at 0x1579c..0x15828.
 */

#include "types.h"
#include "globals.h"
#include "sprglobs.h"
#include "sprload.h"


/* sp_fidx (Ghidra sprite_file_index_table @ 0x2A084, 50 shorts):
   file-record index -> sprite_id slot to store its pointers in. */
short   sp_fidx[50] = {
        12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27,
        28, 29, 30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43,
        44,  9, 45, 46, 47, 48, 49,  3,
         4, 50,  7,  6, 51, 52, 53, 54,
         8, 55
};

/* sp_mbuf: 14 KB shared mask buffer.  sp_regs writes a generated
   transparency mask here parallel to the sprite's image bytes in
   spr_file.  g_sedms[id] then references a slice
   here. */
unsigned char   sp_mbuf[14000];

/* sp_genma -> parts/sp_genma.c (STX: 0x408c, right after cntSong). */
#ifdef FAITHFUL
#include "parts/sp_genma.c"
#endif

/* sp_regs -> parts/sp_regs.c (STX: 0x5bdc, in the 0x400c object between lc_load and gameLoop). */
#ifdef FAITHFUL
#include "parts/sp_regs.c"
#endif

/* sp_reglp does not exist in LCP_STX: main inlines its loop and
   calls sp_regs (parts/sp_regs.c) per sprite. */
