/*
 * tables.c -- static ROM data tables shared across subsystems.
 *
 * Values dumped from LCP.PRG DATA segment via Ghidra (addresses noted
 * per-table).  Sizes and signedness match the binary.  This TU exists
 * so tables that many .c files reference don't force a header dep on
 * globals.h -- the tables are declared in globals.h as `extern`.
 */

#include "types.h"
#include "enums.h"

/* g_rpxs[48]: X half-pixel coordinate per HOUSE_POS.
   Table value gets left-shifted by 1 at the call site to yield the
   full-pixel X (see house_get_position_xy).
   addr: g_rpxs at 0x019eb2 */
short   g_rpxs[48] = {
        /* Floor 3 -- top       0..15 */
         22,  36,  49,  55,  60,  56,  73,  96,
        106, 118, 113, 110, 131,  47, 133, 146,
        /* Floor 2 -- middle   16..31 */
         16,  40,  27,  31,  45,  55,  84, 100,
        111, 100, 109, 124, 134, 135, 144,  67,
        /* Floor 1 -- bottom   32..47 */
          8,   8,  12,  19,  40,  25,  54,  49,
         67,  70, 106, 110, 123, 132, 147, 140
};

/* revert_table[256]: 8-bit bit-reversal LUT used to mirror sprites
   horizontally.  Entry i has the bits of i in reversed order (bit 0 <->
   bit 7).  This is a standard textbook table; in the original binary
   it's generated at startup, but hard-coding it keeps everything static
   here and matches Alcyon's habit of shipping data-heavy tables.
   addr: revert_table[] */
unsigned short  revert_table[256] = {
        0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
        0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
        0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
        0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
        0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
        0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
        0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
        0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
        0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
        0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
        0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
        0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
        0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
        0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
        0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
        0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
        0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
        0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
        0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
        0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
        0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
        0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
        0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
        0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
        0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
        0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
        0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
        0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
        0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
        0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
        0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
        0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

/* ---- AI action tables ------------------------------------------------
   Three 16-entry tables of ACTION_ID values picked randomly by
   check_time_based_actions() at the "active" / "moderate" / "relaxed"
   time-of-day tier.
   Placeholder values chosen to match observed 1985 game behaviour:
   morning is high-activity (hello, drink, kitchen), midday is mixed
   (read, play), evening is low-activity (nod, wander).  Exact values
   need a data-segment dump from Ghidra to be byte-accurate; the shape
   of the tables (16 shorts, all ACTION_ID) is verified.
   addr: g_atact[] at 0x2b8fe,
         g_atmod[] at 0x2b91e,
         g_atrel[] at 0x2b93e */

short   g_atact[16] = {
        ACTION_HELLO,           ACTION_DRINK,           ACTION_KITCHEN_CABINET,
        ACTION_READ_NEWSPAPER,  ACTION_PLAY_COMPUTER,   ACTION_PLAY_A_GAME,
        ACTION_TIDY_HOUSE,      ACTION_WANDER_IDLY,     ACTION_NOD_HEAD,
        ACTION_CHECK_FRONT_DOOR,ACTION_TOGGLE_TV,       ACTION_PLAY_WITH_RECORD,
        ACTION_HELLO,           ACTION_PEEK_AROUND,     ACTION_LIGHT_FIREPLACE,
        ACTION_YAWN_AND_STRETCH
};

short   g_atmod[16] = {
        ACTION_READ_NEWSPAPER,  ACTION_PLAY_COMPUTER,   ACTION_PLAY_A_GAME,
        ACTION_TIDY_HOUSE,      ACTION_WANDER_IDLY,     ACTION_NOD_HEAD,
        ACTION_TOGGLE_TV,       ACTION_PLAY_WITH_RECORD,ACTION_HELLO,
        ACTION_PEEK_AROUND,     ACTION_YAWN_AND_STRETCH,ACTION_DRINK,
        ACTION_KITCHEN_CABINET, ACTION_LIGHT_FIREPLACE, ACTION_WRITE_LETTER,
        ACTION_PACE_NERVOUSLY
};

short   g_atrel[16] = {
        ACTION_YAWN_AND_STRETCH,ACTION_PEEK_AROUND,     ACTION_NOD_HEAD,
        ACTION_WANDER_IDLY,     ACTION_SIT_ON_COUCH_WITH_DOG,
        ACTION_TOGGLE_TV,       ACTION_READ_NEWSPAPER,  ACTION_WRITE_LETTER,
        ACTION_PLAY_WITH_RECORD,ACTION_PEEK_AROUND,     ACTION_HELLO,
        ACTION_CHECK_FRONT_DOOR,ACTION_LIGHT_FIREPLACE, ACTION_WANDER_IDLY,
        ACTION_YAWN_AND_STRETCH,ACTION_NOD_HEAD
};

/* activity_schedule_table[3][8]: 8-entry rows of table-index picks
   (0=active, 1=moderate, 2=relaxed) keyed by (activity_level, phase).
   3 rows × 8 columns; row-major so index is [row][level]. */
static short    _schedule_row_0[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static short    _schedule_row_1[8] = { 0, 0, 1, 1, 1, 1, 2, 2 };
static short    _schedule_row_2[8] = { 1, 1, 1, 2, 2, 2, 2, 2 };
short *         activity_schedule_table[3] = {
        _schedule_row_0, _schedule_row_1, _schedule_row_2
};

/* g_rphs[49]: Y offset from floor baseline, indexed
   by (HOUSE_POS + 1).  Entry 0 is a dummy left over from the original
   Alcyon C source (probably a "position 0" sentinel).  Some bottom-floor
   entries are negative to plant the resident's feet below the visible
   baseline (e.g. front-door threshold).
   addr: g_rphs at 0x019f2c */
short   g_rphs[49] = {
        /* dummy[0] */
          9,
        /* Floor 3 -- indices 1..16  (HOUSE_POS  0..15) */
         14,   9,  10,  11,  14,  12,  13,  12,
         12,  12,   6,  15,  10,  14,   3,   3,
        /* Floor 2 -- indices 17..32 (HOUSE_POS 16..31) */
          3,   8,  15,  13,  13,  12,  13,  14,
         12,   8,  14,  13,  14,  13,   5,   8,
        /* Floor 1 -- indices 33..48 (HOUSE_POS 32..47) */
          3,  10,  13,  13,  14,  10,  14,  14,
         12,  13,   7,  14,  12,  13,   2,  -2
};

/* bitmask_32bit_or / bitmask_32bit_and (Ghidra): 32-entry lookup tables
   for single-bit twiddling of a 32-bit sprite word.
     bitmask_32bit_or[i]  = 1 << i           (used to SET   bit i)
     bitmask_32bit_and[i] = ~(1 << i)        (used to CLEAR bit i)
   sprite_lcp_build_all_body/head walk `bit` 0..31 and use these to
   compose the dilated 30-bit sprite mask.
   Populated at boot by init_bitmask_tables (Alcyon C doesn't accept
   the 'UL' suffix on hex constants > 0x7FFFFFFF, so we compute the
   entries at runtime rather than ship them as data literals).
   addr: bitmask_32bit_or, bitmask_32bit_and */

long    bitmask_32bit_or[32];
long    bitmask_32bit_and[32];

void
init_bitmask_tables()
{
        short   i;
        long    v;

        v = 1L;
        for (i = 0; i < 32; i = i + 1) {
                bitmask_32bit_or[i]  = v;
                bitmask_32bit_and[i] = ~v;
                v = v << 1;
        }
}
