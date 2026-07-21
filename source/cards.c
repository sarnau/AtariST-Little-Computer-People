/*
 * cards.c -- CARDS graphics file loader.
 *
 * The CARDS file on the 1985 disk holds 53 16x24 pixel card bitmaps
 * in 4-bitplane low-res format (192 bytes per card): 12 face cards
 * per suit for 4 suits, then 1 shared card back at the end.  We add
 * one more synthetic entry (index 54) as a black-and-white highlight
 * overlay used to indicate the currently-selected card in the poker
 * hand draw UI.
 *
 * Layout inside crd_dat (unit = 16-bit word; each card = 96 words):
 *
 *   +---------------+---------------+---------------+---------------+
 *   |  suit 0       |  suit 1       |  suit 2       |  suit 3       |  suit trailer  back  highlight
 *   |  0x000..0x480 |  0x4e0..0x960 |  0x9c0..0xe40 |  0xea0..0x1320|    0x1380      0x13e0
 *   | 12 face cards | 12 face cards | 12 face cards | 12 face cards |    * 4         (52)     (53)
 *   +---------------+---------------+---------------+---------------+
 *
 * Faces are stored per-suit in *reverse rank order* (Ace at offset
 * 11*0x60, King at 0*0x60), because the game's card indexing is
 * high-first.  A trailer slot at 0x480 words into each suit holds a
 * per-suit "suit-only" card back (probably used during blind deals).
 * The single global back at crd_dat + 0x1380 is the standard
 * face-down card.
 *
 * addr: pk_ldCrd()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "cards.h"
#include "globals.h"
#include "save.h"
#include "sprender.h"


/* pk_ldCrd: read 53 card bitmaps from disk into
   crd_dat, synthesise the 54th highlight pattern, then wire all 54
   MFDB descriptors + the destination-screen MFDB used by the game UI.

   addr: pk_ldCrd() */

void
pk_ldCrd()
{
        short           fhnd;
        short           suit;
        short           rank;
        short           i;
        unsigned short *buf;

        fhnd = fOpen("cards", 0);
        buf = (unsigned short *) crd_dat;

        /* 4 suits x (12 face cards reverse-ranked + 1 per-suit back). */
        for (suit = 0; suit < 4; suit = suit + 1) {
                for (rank = 0; rank < 12; rank = rank + 1) {
                        fr_read(fhnd, 0xc0L,
                                  buf + suit * 0x4e0 +
                                        (short) (11 - rank) * 0x60);
                }
                /* Per-suit trailer slot at word offset 0x480. */
                fr_read(fhnd, 0xc0L,
                          buf + suit * 0x4e0 + 0x480);
        }

        /* Standard face-down back at slot 52. */
        fr_read(fhnd, 0xc0L, crd_dat + 0x1380);
        Fclose(fhnd);

        /* Synthesize the highlight overlay at slot 53: a 4-plane
           pattern with plane 0 blank, plane 1 blank, planes 2 and 3
           solid.  In the low-res palette this shades everything a
           uniform light colour that overlays cleanly on face-up
           cards to mark the current selection. */
        for (i = 0x13e0; i < 0x1440; i = i + 4) {
                crd_dat[i]     = 0;
                crd_dat[i + 1] = 0;
                crd_dat[i + 2] = (short) 0xffff;
                crd_dat[i + 3] = (short) 0xffff;
        }

        /* Wire the 54 MFDB descriptors: each points 96 words further
           into crd_dat than the previous, dimensions 16 wide by
           24 tall. */
        for (i = 0; i < 54; i = i + 1)
                sp_iniM(0L, &crd_mfdb[i],
                                 crd_dat + i * 0x60,
                                 (short) 16, (short) 24);

        /* Destination MFDB for the game display area (full-width strip
           of 320 pixels by 77 rows). */
        sp_iniM(0L, &mf_scb_c,
                         g_dscp,
                         (short) 320, (short) 77);
}
