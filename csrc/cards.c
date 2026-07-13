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
 * Layout inside cards_data (unit = 16-bit word; each card = 96 words):
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
 * The single global back at cards_data + 0x1380 is the standard
 * face-down card.
 *
 * addr: poker_load_card_graphics()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern void *   dest_screenbase_ptr;
extern short *  cards_data;
extern MFDB     cards_MFDB_blocks[];
extern MFDB     MFDB_dest_screenbase_cards;
#include <osbind.h>

extern short    file_open();
extern void     fr_read();
extern void     sp_iniM();

/* poker_load_card_graphics: read 53 card bitmaps from disk into
   cards_data, synthesise the 54th highlight pattern, then wire all 54
   MFDB descriptors + the destination-screen MFDB used by the game UI.

   addr: poker_load_card_graphics() */

void
poker_load_card_graphics()
{
        short           fileHandle;
        short           suit;
        short           rank;
        short           i;
        unsigned short *buf;

        fileHandle = file_open("cards", 0);
        buf = (unsigned short *) cards_data;

        /* 4 suits x (12 face cards reverse-ranked + 1 per-suit back). */
        for (suit = 0; suit < 4; suit = suit + 1) {
                for (rank = 0; rank < 12; rank = rank + 1) {
                        fr_read(fileHandle, 0xc0L,
                                  buf + suit * 0x4e0 +
                                        (short) (11 - rank) * 0x60);
                }
                /* Per-suit trailer slot at word offset 0x480. */
                fr_read(fileHandle, 0xc0L,
                          buf + suit * 0x4e0 + 0x480);
        }

        /* Standard face-down back at slot 52. */
        fr_read(fileHandle, 0xc0L, cards_data + 0x1380);
        _gemdos(GEMDOS_Fclose, (long) fileHandle, 0L, 0L);

        /* Synthesize the highlight overlay at slot 53: a 4-plane
           pattern with plane 0 blank, plane 1 blank, planes 2 and 3
           solid.  In the low-res palette this shades everything a
           uniform light colour that overlays cleanly on face-up
           cards to mark the current selection. */
        for (i = 0x13e0; i < 0x1440; i = i + 4) {
                cards_data[i]     = 0;
                cards_data[i + 1] = 0;
                cards_data[i + 2] = (short) 0xffff;
                cards_data[i + 3] = (short) 0xffff;
        }

        /* Wire the 54 MFDB descriptors: each points 96 words further
           into cards_data than the previous, dimensions 16 wide by
           24 tall. */
        for (i = 0; i < 54; i = i + 1)
                sp_iniM(0L, &cards_MFDB_blocks[i],
                                 cards_data + i * 0x60,
                                 (short) 16, (short) 24);

        /* Destination MFDB for the game display area (full-width strip
           of 320 pixels by 77 rows). */
        sp_iniM(0L, &MFDB_dest_screenbase_cards,
                         dest_screenbase_ptr,
                         (short) 320, (short) 77);
}
