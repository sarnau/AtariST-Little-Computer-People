/*
 * Must sit right before dispCompChips; playPoker and playWar call it.
 *
 * Included by games.c; never compiled on its own.
 */

/*
 * CARDS graphics file loader.
 *
 * The CARDS file on the 1985 disk holds 53 16x24 pixel card bitmaps
 * in 4-bitplane low-res format (192 bytes per card): 12 face cards
 * per suit for 4 suits, then 1 shared card back at the end.  We add
 * one more synthetic entry (index 54) as a black-and-white highlight
 * overlay used to indicate the currently-selected card in the poker
 * hand draw UI.
 *
 * Layout inside cardImages (unit = 16-bit word; each card = 96 words):
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
 * The single global back at cardImages + 0x1380 is the standard
 * face-down card.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "protos.h"
#include "globals.h"


/* Loads the CARDS file into cardImages in the layout described above,
   builds the selection-highlight card at slot 53 itself, and points the
   54 card MFDBs and the card-table MFDB cardTableMfdb (the 320x77 area at
   stripBuf) at their bitmaps.  Called by the card games before play. */
void
cardLoad()
{
        /* Four locals; the first is reused as the index of both
           trailing loops. */
        short   rank;
        short   suit;
        short   fhnd;
        char *  buf;

        fhnd = openFile("cards", RMODE_RD);
        buf  = (char *) cardImages;

        /* 4 suits x (12 face cards reverse-ranked + 1 per-suit back).
           The offsets are BYTE offsets added to a char*, with the
           trailing constant folded into the argument slot. */
        for (suit = 0; suit < 4; suit++) {
                for (rank = 0; rank < 12; rank++)
                        readFile(fhnd, 0xc0L,
                                (long) (suit * 2496) +
                                (11 - rank) * 192 + buf);
                /* Per-suit trailer slot. */
                readFile(fhnd, 0xc0L, (long) (suit * 2496) + buf + 2304);
        }

        /* Standard face-down back at slot 52. */
        readFile(fhnd, 0xc0L, (char *) cardImages + 9984);
        Fclose(fhnd);

        /* Synthesize the highlight overlay at slot 53: planes 0 and 1
           blank, planes 2 and 3 solid. */
        for (rank = 0x13e0; rank < 0x1440; ) {
                cardImages[rank] = 0;
                rank++;
                cardImages[rank] = 0;
                rank++;
                cardImages[rank] = -1;
                rank++;
                cardImages[rank] = -1;
                rank++;
        }

        /* Wire the 54 MFDB descriptors. */
        for (rank = 0; rank < 54; rank++)
                initMfdb(0L, &cardMfdb[rank],
                        (long) (rank * 192) + (char *) cardImages, 16, 24);

        initMfdb(0L, &cardTableMfdb, stripBuf, 320, 77);
}
