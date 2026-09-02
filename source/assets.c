/*
 * assets.c -- OBJECTS/SPRITES/BODY.LCP/PEx.LCP/NAMES loaders and
 * the dispatchers that unpack them into runtime MFDB tables.
 *
 * OBJECTS/SPRITES record: {h:BE16, w:BE16, ceil(w/16)*4*2*h pixel bytes}
 *   (4 bitplanes interleaved per row, MSB-first).  File caps at 14000.
 * BODY.LCP / PE2..PE6.LCP: {count:BE16, total_bytes:BE16, payload}
 *   168 bytes per 16x21 frame (21 rows x 4 words = 2 image + 2 mask).
 * NAMES: newline-terminated ASCII, <= 10 chars per line.
 *
 * addr: ldObj(), ldSpr()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "alerts.h"
#include "assets.h"
#include "globals.h"
#include "save.h"
#include "sprender.h"
#include "sprglobs.h"
#include "sprites.h"


/* ldObj -> parts/ldObj.c (STX: 0x524a). */
#ifdef FAITHFUL
#include "parts/ldObj.c"
#endif

/* ldSpr -> parts/ldSpr.c (STX: 0x528a). */
#ifdef FAITHFUL
#include "parts/ldSpr.c"
#endif

/* Parse OBJECTS/SPRITES buffer -> per-record MFDB + w/h arrays.
   Stops at buffer end / height==0 / 64 records. */
/* prsRec does not exist in LCP_STX: main inlines it. */

/* al_loot: read OBJECTS and unpack.  Port-side wrapper; ROM inlines
   at 0x15546 as ldObj() + 56-iter parse loop. */
/* al_loot does not exist in LCP_STX: main inlines it. */

/* al_lost: read SPRITES and unpack.  Port-side wrapper. */
/* al_lost does not exist in LCP_STX: main inlines it. */

/* al_loal -> parts/al_loal.c (STX: 0x6428, in the 0x400c object ahead of fl_ltpl). */
#ifdef FAITHFUL
#include "parts/al_loal.c"
#endif

/* al_locs: load BODY.LCP + PEx.LCP (x = character_sprite_id, 2..6,
   clamped to 2).  Wires body_ptr and pex_ptr.  Static buffers
   (survive to game end without heap fragmentation). */

/* body.lcp @ 0x3f8b0 = 20160 B, pex_lcp_file @ 0x4d2da = 11088 B
   (168 bytes/frame, sp_lcpf w=2/h=21). */
/* LCP_STX reads both files straight into the global frame arrays
   (body_ptr / pex_ptr), so there are no staging buffers. */
/* al_locs does not exist in LCP_STX: main inlines it. */

/* unScn: decode .SCN screen image into out_wds (16-bit words).
   Nibble-stream like fr_reac, but 15-WORD dictionary at file offset
   2..31 (30 bytes), 0xF escape reads 4 more nibbles for literal word.
   Header 32 bytes; payload at 32.
   addr: decompress_scn @ ROM 0x15546 (with wrapper fOpen/Malloc/etc). */
/* unScn does not exist in LCP_STX: main inlines it. */

/* al_loan: read NAMES text file (plain ASCII, newline-terminated names,
   ~2.6 KB on the 1985 disk). */

long
al_loan(dest_buf, max_b)
unsigned char * dest_buf;
long            max_b;
{
        short   fhnd;

        fhnd = fOpen("names", 0);
        fr_read(fhnd, max_b, dest_buf);
        Fclose(fhnd);
        return max_b;
}
