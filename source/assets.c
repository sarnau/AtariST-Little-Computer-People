/*
 * assets.c -- OBJECTS/SPRITES/BODY.LCP/PEx.LCP/NAMES loaders and
 * the dispatchers that unpack them into runtime MFDB tables.
 *
 * OBJECTS/SPRITES record: {h:BE16, w:BE16, ceil(w/16)*4*2*h pixel bytes}
 *   (4 bitplanes interleaved per row, MSB-first).  File caps at 14000.
 * BODY.LCP / PE2..PE6.LCP: {count:BE16, total_bytes:BE16, payload}
 *   168 bytes per 16x21 frame (21 rows x 4 words = 2 image + 2 mask).
 *   BODY.LCP is 20160 bytes, a PEx.LCP 11088.
 * NAMES: newline-terminated ASCII, <= 10 chars per line.
 * .SCN: nibble stream like fr_reac's, but with a 15-WORD dictionary in
 *   bytes 2..31 of the 32-byte header; nibble 0xF escapes to 4 more
 *   nibbles forming a literal word.  Payload starts at 32.
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


/* ldObj -> parts/ldObj.c. */

/* ldSpr -> parts/ldSpr.c. */

/* al_loal -> parts/al_loal.c. */

/* There are no separate parse/load wrappers here: main itself unpacks
   OBJECTS and SPRITES (stopping at buffer end, height 0 or 64
   records), reads BODY.LCP and PEx.LCP (x = character_sprite_id,
   2..6, clamped to 2) straight into the global frame arrays body_ptr
   and pex_ptr, and decodes the .SCN screen.  lcp_crnd reads NAMES
   itself (Fseek to a random 10-byte record). */
