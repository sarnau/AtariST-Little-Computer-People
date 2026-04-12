"""
Natural-language command parser for Little Computer People (Atari ST).
Extends the word-action mapping from LCP.py with GameState integration.

addr: string_input(), parse_player_command(), enteredword_to_action[]

The player types English phrases which are parsed against a 161-word vocabulary.
Each recognised word is mapped to a bit in a 10-byte bitmask.
The bitmask is compared against sentence_to_action[] patterns to find
the closest matching action and its priority.

Source data from LCP.py (unchanged):
  words[]              — 161-word vocabulary
  words_to_byte_offset[] — which byte (0–9) each word's bit lives in
  word_to_bitnumber[]  — which bit within that byte
  sentence_to_action[] — 32 patterns × 12 bytes (10 mask bytes + action + priority)
"""

from typing import Optional
from .enums import ACTION_ID, WORD_ID
from .state import GameState


# ---------------------------------------------------------------------------
# Vocabulary and encoding tables
# (verbatim from LCP.py — do not modify)
# ---------------------------------------------------------------------------

WORDS_TO_BYTE_OFFSET = [
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 4, 4,
    4, 4, 4, 4, 4, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 9, 9, 9, 9, 9,
]

WORD_TO_BITNUMBER = [
    3, 0, 1, 2, 2, 4, 4, 5, 5, 5,
    5, 5, 6, 6, 6, 6, 6, 6, 7, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 1, 2,
    2, 2, 3, 3, 3, 3, 3, 4, 5, 5,
    5, 6, 7, 7, 7, 0, 0, 1, 1, 1,
    1, 1, 1, 1, 1, 2, 3, 3, 3, 3,
    4, 4, 5, 5, 6, 7, 0, 1, 2, 2,
    2, 3, 4, 4, 4, 5, 5, 7, 1, 1,
    2, 2, 2, 2, 3, 0, 0, 1, 1, 1,
    1, 1, 2, 2, 2, 3, 3, 4, 4, 4,
    4, 4, 4, 5, 5, 5, 5, 5, 5, 5,
    6, 0, 0, 1, 1, 2, 2, 2, 2, 3,
    3, 3, 4, 5, 6, 6, 7, 7, 7, 7,
    7, 0, 1, 2, 3, 4, 5, 6, 7, 7,
    0, 1, 2, 2, 2, 2, 3, 3, 4, 4,
    4, 4, 4, 4, 4, 1, 1, 1, 1, 1,
]

WORDS = [
    "PLEASE", "DO", "YOU", "LIKE", "ENJOY", "WILL", "WOULD", "PLAY",
    "PERFORM", "USE", "TRY", "PLAYING", "ALLERGY", "ALLERGIC", "FEVER",
    "DUST", "POLLEN", "HANKY", "RELAX", "LIGHT", "START", "MAKE", "BURN",
    "IGNITE", "BUILD", "LOOKS", "IS", "SEEMS", "APPEARS", "SEEM", "LOOK",
    "APPEAR", "HEAR", "LISTEN", "PUT", "START", "SPIN", "ON", "CLEAN",
    "TIDY", "PICK", "UP", "SLOPPY", "MESSY", "UNTIDY", "SHOULD", "OUGHT",
    "PROGRAM", "UTILITIES", "MATH", "HOMEWORK", "ADD", "SUBTRACT",
    "MULTIPLY", "DIVIDE", "TICKLE", "TYPE", "TELL", "WRITE", "CONFIDE",
    "BRUSH", "FLOSS", "DRINK", "IMBIBE", "GET", "FEED", "FILL", "OPEN",
    "DANCE", "MOON", "SHOW", "LIKE", "TIRED", "BORED", "APATHETIC",
    "HATE", "AWFUL", "IF", "WHAT", "WHAT'S", "IN", "INSIDE", "STORED",
    "KEEP", "IS", "PIANO", "ORGAN", "STEREO", "TURNTABLE", "MUSIC",
    "RECORD", "PLATTER", "FIRE", "FIREPLACE", "LOG", "CHILLY", "COLD",
    "PROBLEM", "PROBLEMS", "TROUBLES", "MATTER", "LETTER", "NOTE", "SONG",
    "TUNE", "SONATA", "FUGUE", "SERENADE", "JAZZ", "BOOGIE", "IVORIES",
    "TEETH", "HYGIENE", "GLASS", "COOLER", "DOG", "PET", "MUTT", "POOCH",
    "BOWL", "DISH", "CAN", "TV", "CHAIR", "COMPUTER", "ATARI", "WATER",
    "LIQUID", "LIQUIDS", "FLUID", "FLUIDS", "UPSTAIRS", "BEDROOM",
    "CLOSET", "KITCHEN", "FILING", "CABINET", "FREEZER", "REFRIDGERATOR",
    "FRIDGE", "DRESSER", "NIGHTSTAND", "ADDITION", "SUBTRACTION",
    "MULTIPLICATION", "DIVISION", "HOUSE", "HOME", "GAME", "CARDS",
    "POKER", "WAR", "CARD", "ANAGRAMS", "BLACKJACK", "EXCUSE", "PARDON",
    "HELLO", "ATTENTION", "HEY",
]

# sentence_to_action[]: 32 patterns × 12 bytes
# Bytes 0–9: word bitmask (10 bytes)
# Byte 10:   ACTION_ID
# Byte 11:   priority weight
SENTENCE_TO_ACTION = [
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x18, 15,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x14,  4,
    0x02, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x14,  2,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x14,  4,
    0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x05,  4,
    0x00, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24,  8,
    0x00, 0x80, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x24,  2,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x1A,  4,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x1A,  4,
    0x00, 0x00, 0x04, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x1A,  4,
    0x00, 0x00, 0x08, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x07,  8,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x07,  6,
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x11,  2,
    0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x11,  2,
    0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x0D,  2,
    0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x0D,  4,
    0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x1F,  8,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1F,  8,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1F,  8,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,  2,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x06,  8,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x06,  8,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x10,  8,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0E,  6,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x02,  6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x05, 0x00, 0x00, 0x1B,  6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x22,  6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x28, 0x00, 0x00, 0x12,  6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x30, 0x00, 0x00, 0x10,  6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x40, 0x00, 0x00, 0x12,  6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x80, 0x00, 0x00, 0x12,  6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x22,  6,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x02, 0x00, 0x22,  6,
]


# ---------------------------------------------------------------------------
# Text → word bitmask encoding
# addr: parse_player_command() / input processing
# ---------------------------------------------------------------------------

def text_to_word_bits(text: str) -> list[int]:
    """
    Convert a player-typed phrase into a 10-byte word bitmask.
    Each recognised vocabulary word sets a specific bit in the 10-byte array.
    addr: (input processing that builds entered_word_bits[10])
    """
    bits = [0] * 10
    text_upper = text.upper()
    for word_idx, word in enumerate(WORDS):
        if word in text_upper:
            byte_offset = WORDS_TO_BYTE_OFFSET[word_idx]
            bit_number  = WORD_TO_BITNUMBER[word_idx]
            bits[byte_offset] |= (1 << bit_number)
    return bits


def match_action(word_bits: list[int]) -> tuple[int, int]:
    """
    Find the best matching action for a given word bitmask.
    Returns (action_id, priority) or (ACTION_NONE, 0) if no match.
    addr: sentence matching against sentence_to_action[]
    """
    best_action   = ACTION_ID.ACTION_NONE
    best_priority = 0
    best_match_count = 0

    for pattern_offset in range(0, len(SENTENCE_TO_ACTION), 12):
        pattern       = SENTENCE_TO_ACTION[pattern_offset:pattern_offset + 10]
        action_id     = SENTENCE_TO_ACTION[pattern_offset + 10]
        priority      = SENTENCE_TO_ACTION[pattern_offset + 11]

        # Count matching bits: pattern bits must be a subset of entered bits
        match_count = 0
        all_matched = True
        for i in range(10):
            required = pattern[i]
            if required == 0:
                continue
            if (word_bits[i] & required) != required:
                all_matched = False
                break
            match_count += bin(required).count('1')

        if all_matched and match_count > 0:
            if match_count > best_match_count or (
                match_count == best_match_count and priority > best_priority
            ):
                best_match_count = match_count
                best_action      = action_id
                best_priority    = priority

    return best_action, best_priority


# ---------------------------------------------------------------------------
# Public API — parse a player command and add to the action queue
# addr: string_input() + command dispatch
# ---------------------------------------------------------------------------

def parse_and_queue_command(gs: GameState, text: str) -> Optional[tuple[int, int]]:
    """
    Parse a player-typed phrase and enqueue the resulting action.
    Returns (action_id, priority) if a match was found, else None.

    Integration with GameState:
      - Sets gs.entered_word_bits from the parsed text
      - Calls ai.add_command_to_queue() with the matched action
    """
    word_bits = text_to_word_bits(text)
    gs.entered_word_bits = word_bits

    action_id, priority = match_action(word_bits)
    if action_id == ACTION_ID.ACTION_NONE:
        return None

    from .ai import add_command_to_queue
    add_command_to_queue(gs, action_id, priority)
    return action_id, priority


def decode_all_patterns() -> list[tuple[int, int, list[str]]]:
    """
    Decode all sentence_to_action patterns into human-readable form.
    Returns list of (action_id, priority, [word_groups]) for inspection.
    Ported verbatim from LCP.py.
    """
    result = []
    for pattern_offset in range(0, len(SENTENCE_TO_ACTION), 12):
        action_id = SENTENCE_TO_ACTION[pattern_offset + 10]
        priority  = SENTENCE_TO_ACTION[pattern_offset + 11]
        word_groups: list[str] = []
        for boffset in range(10):
            for bitpos in range(7):
                if SENTENCE_TO_ACTION[pattern_offset + boffset] & (1 << bitpos):
                    group = []
                    for wpos, word in enumerate(WORDS):
                        if (WORDS_TO_BYTE_OFFSET[wpos] == boffset
                                and WORD_TO_BITNUMBER[wpos] == bitpos):
                            group.append(word)
                    if group:
                        word_groups.append('|'.join(group))
        result.append((action_id, priority, word_groups))
    return result
