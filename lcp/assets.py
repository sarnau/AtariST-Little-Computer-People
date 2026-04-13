"""
Asset loader for Little Computer People (Atari ST).
Extends readFiles.py with a unified interface that loads all game data
files into memory at startup and populates the sprite pipeline in GameState.

All file paths are resolved relative to a configurable DATA_DIR (default: ./DATA).
"""

import os
import math
import struct
import binascii
from pathlib import Path
from typing import Optional

from PIL import Image, ImageDraw

from .enums import SPRITE_ID
from .structs import LCP, PSG_ENVELOPE, FILE_IMG_DATA
from .constants import SPRITEDATA_INDEX_TABLE, ATARI_PALETTE_RGB
from .state import GameState


# ---------------------------------------------------------------------------
# Default data directory (same layout as the original game distribution)
# ---------------------------------------------------------------------------
DEFAULT_DATA_DIR = Path(__file__).parent.parent / 'DATA'


# ---------------------------------------------------------------------------
# Atari ST 4-bitplane pixel format helpers
# (ported from readFiles.py)
# ---------------------------------------------------------------------------

def _st_pixel_to_rgba(planes: tuple[int, int, int, int], pixel: int,
                       transparent: bool) -> tuple[int, int, int, int]:
    """Extract one pixel's colour index from four bitplane words and map to RGBA."""
    c0 = (planes[0] >> (15 - pixel)) & 1
    c1 = (planes[1] >> (15 - pixel)) & 1
    c2 = (planes[2] >> (15 - pixel)) & 1
    c3 = (planes[3] >> (15 - pixel)) & 1
    col = (c0) | (c1 << 1) | (c2 << 2) | (c3 << 3)
    alpha = 255 if (not transparent or col != 0) else 0
    r, g, b = ATARI_PALETTE_RGB[col]
    return (r, g, b, alpha)


def decode_st_bitmap(width: int, height: int, data: bytes,
                     offset: int = 0, transparent: bool = False) -> Image.Image:
    """
    Decode an Atari ST 4-bitplane interleaved bitmap to a PIL Image.
    Returns an RGBA image.  transparent=True makes colour-0 pixels transparent.
    """
    img = Image.new('RGBA', (width, height), (255, 255, 255, 255))
    draw = ImageDraw.Draw(img)
    words_per_row = math.ceil(width / 16)
    for row in range(height):
        for word in range(words_per_row):
            planes = struct.unpack_from('>4H', data, offset)
            offset += 8
            for pixel in range(16):
                x = pixel + word * 16
                if x >= width:
                    continue
                rgba = _st_pixel_to_rgba(planes, pixel, transparent)
                draw.point((x, row), rgba)
    return img


# ---------------------------------------------------------------------------
# Nibble-dict decompression (from readFiles.py)
# Used for TEXT files (WORDS, WORDPZ.TXT, LETTER.TXT) and image files (.SCN)
# ---------------------------------------------------------------------------

def decompress_text_file(path: Path) -> str:
    """
    Decompress a nibble-encoded text file (WORDS, WORDPZ.TXT, LETTER.TXT).
    addr: decompressFile() in readFiles.py
    """
    data = path.read_bytes()
    file_size, = struct.unpack('>H', data[0:2])
    header = data[2:17]
    body = data[17:]
    offset = 0
    flag = True
    output = []
    count = 0
    while offset < file_size - 17:
        if flag:
            nibble = (body[offset] >> 4) & 0xF
            flag = False
        else:
            nibble = body[offset] & 0xF
            offset += 1
            flag = True
        if nibble != 0xF:
            output.append(chr(header[nibble]))
        else:
            c = 0
            for _ in range(2):
                c <<= 4
                if flag:
                    c |= (body[offset] >> 4) & 0xF
                    flag = False
                else:
                    c |= body[offset] & 0xF
                    offset += 1
                    flag = True
            output.append(chr(c))
        count += 1
    return ''.join(output)


def decompress_image_file(path: Path) -> bytes:
    """
    Decompress a nibble-encoded image file (.SCN screens).
    Returns raw decompressed bytes (Atari ST 4-bitplane words as hex → binary).
    addr: decompressImageFile() in readFiles.py
    """
    data = path.read_bytes()
    file_size, = struct.unpack('>H', data[0:2])
    header = struct.unpack('>15H', data[2:2 + 15 * 2])
    body = data[32:]
    offset = 0
    flag = True
    hex_output = []
    while offset < file_size - 32:
        if flag:
            nibble = (body[offset] >> 4) & 0xF
            flag = False
        else:
            nibble = body[offset] & 0xF
            offset += 1
            flag = True
        if nibble != 0xF:
            hex_output.append('%04x' % header[nibble])
        else:
            c = 0
            for _ in range(4):
                c <<= 4
                if flag:
                    c |= (body[offset] >> 4) & 0xF
                    flag = False
                else:
                    c |= body[offset] & 0xF
                    offset += 1
                    flag = True
            hex_output.append('%04x' % c)
    return binascii.unhexlify(''.join(hex_output))


# ---------------------------------------------------------------------------
# Sprite / object file loader
# ---------------------------------------------------------------------------

def load_sprite_file(path: Path, transparent: bool = True
                     ) -> dict[int, tuple[Image.Image, bytes, bytes, int, int]]:
    """
    Load the SPRITES or OBJECTS binary file.
    Returns a dict mapping logical SPRITE_ID → (pil_image, pixel_data, mask_data, width, height).

    File format: sequence of FILE_IMG_DATA header (4 bytes) + pixel data.
    The 50 sprites in file order are remapped to logical slots via SPRITEDATA_INDEX_TABLE.
    addr: spritedata_load() / loadSpritesOrObjects() in readFiles.py
    """
    raw = path.read_bytes()
    offset = 0
    file_index = 0
    result: dict[int, tuple] = {}

    while offset < len(raw) and file_index < len(SPRITEDATA_INDEX_TABLE):
        hdr = FILE_IMG_DATA.from_bytes(raw[offset:])
        offset += FILE_IMG_DATA._PACK_SIZE
        byte_count = hdr.byte_size
        pixel_data = raw[offset:offset + byte_count]
        offset += byte_count

        img = decode_st_bitmap(hdr.width, hdr.height, pixel_data, transparent=transparent)
        # Build 1-bit mask: transparent pixels → 0, opaque → 1
        mask_data = _build_sprite_mask(pixel_data, hdr.width, hdr.height)

        logical_id = SPRITEDATA_INDEX_TABLE[file_index]
        result[logical_id] = (img, pixel_data, mask_data, hdr.width, hdr.height)
        file_index += 1

    return result


def _build_sprite_mask(pixel_data: bytes, width: int, height: int) -> bytes:
    """
    Generate a 1-bit-per-pixel mask from 4-bitplane sprite data.
    Colour 0 (all bitplanes zero) → transparent (mask bit = 0).
    addr: spritedata_generate_mask_from_color()
    """
    words_per_row = math.ceil(width / 16)
    mask_words = []
    offset = 0
    for _ in range(height):
        for _ in range(words_per_row):
            planes = struct.unpack_from('>4H', pixel_data, offset)
            offset += 8
            # mask word: bit set where any bitplane bit is set (i.e. not colour 0)
            mask = planes[0] | planes[1] | planes[2] | planes[3]
            mask_words.append(mask)
    return struct.pack(f'>{len(mask_words)}H', *mask_words)


# ---------------------------------------------------------------------------
# Screen image loader
# ---------------------------------------------------------------------------

def load_screen(path: Path) -> Image.Image:
    """Load and decompress a .SCN compressed background image (320×200)."""
    raw = decompress_image_file(path)
    return decode_st_bitmap(320, 200, raw, transparent=False)


# ---------------------------------------------------------------------------
# Body / head sprite sheet loader (.LCP files)
# ---------------------------------------------------------------------------

def load_lcp_sprite_sheet(path: Path,
                          palette_indices: tuple[int, ...] = (0, 1, 2, 3),
                          ) -> list[Image.Image]:
    """
    Load a .LCP sprite sheet (BODY.LCP, PE2.LCP … PE6.LCP).

    The file stores 32×21 pixel sprites in 2-bitplane interleaved format:
      4 words per row = 2 chunks of 2 words (bitplane 0, bitplane 1)
      Chunk 0: left 16 pixels   (words 0, 1)
      Chunk 1: right 16 pixels  (words 2, 3)

    sprite_lcp_flip() remaps the 2 source bitplanes into different
    destination bitplane positions to select different palette ranges:
      Body  (flipVertical=1): dest bitplanes 0,1 → palette entries 0,1,2,3
      Head  (flipVertical=0): dest bitplanes 1,2 → palette entries 0,2,4,6

    The ``palette_indices`` parameter maps the 4 possible 2-bit colour
    values (0–3) to Atari ST palette indices.

    Returns a list of 32×21 PIL RGBA Images, one per animation frame.
    addr: loadLCP() in readFiles.py, sprite_lcp_flip()
    """
    raw = path.read_bytes()
    _count, _size = struct.unpack('>HH', raw[:4])
    frames: list[Image.Image] = []
    offset = 4
    # Each frame: 21 rows × 4 words × 2 bytes = 168 bytes
    FRAME_BYTES = 21 * 4 * 2
    while offset + FRAME_BYTES <= len(raw):
        img = _decode_lcp_frame(raw, offset, palette_indices)
        frames.append(img)
        offset += FRAME_BYTES
    return frames


def _decode_lcp_frame(data: bytes, offset: int,
                      palette_indices: tuple[int, ...],
                      ) -> Image.Image:
    """
    Decode one 32×21 LCP sprite frame (2-bitplane, 2 chunks per row).
    addr: sprite_lcp_flip() source format
    """
    img = Image.new('RGBA', (32, 21), (0, 0, 0, 0))
    for row in range(21):
        w0, w1, w2, w3 = struct.unpack_from('>4H', data, offset)
        offset += 8
        # Left 16 pixels (chunk 0: bitplane 0 = w0, bitplane 1 = w1)
        for px in range(16):
            bp0 = (w0 >> (15 - px)) & 1
            bp1 = (w1 >> (15 - px)) & 1
            col_idx = bp0 | (bp1 << 1)
            if col_idx != 0:
                pal = palette_indices[col_idx]
                r, g, b = ATARI_PALETTE_RGB[pal]
                img.putpixel((px, row), (r, g, b, 255))
        # Right 16 pixels (chunk 1: bitplane 0 = w2, bitplane 1 = w3)
        for px in range(16):
            bp0 = (w2 >> (15 - px)) & 1
            bp1 = (w3 >> (15 - px)) & 1
            col_idx = bp0 | (bp1 << 1)
            if col_idx != 0:
                pal = palette_indices[col_idx]
                r, g, b = ATARI_PALETTE_RGB[pal]
                img.putpixel((px + 16, row), (r, g, b, 255))
    return img


# ---------------------------------------------------------------------------
# Card sprite loader
# ---------------------------------------------------------------------------

def load_cards(path: Path) -> list[Image.Image]:
    """
    Load 53 playing card images (16×24 pixels each, no transparency).
    addr: loadCards() in readFiles.py
    """
    raw = path.read_bytes()
    cards: list[Image.Image] = []
    offset = 0
    card_byte_size = math.ceil(16 / 16) * 24 * 4 * 2  # 1 word × 24 rows × 4 planes × 2 bytes
    while offset + card_byte_size <= len(raw):
        img = decode_st_bitmap(16, 24, raw, offset=offset, transparent=False)
        cards.append(img)
        offset += card_byte_size
    return cards


# ---------------------------------------------------------------------------
# Save file loader / saver
# ---------------------------------------------------------------------------

def load_hyber(path: Path) -> LCP:
    """
    Load the 128-byte HYBER save file and return an LCP struct.
    addr: main() startup, LCP struct load
    """
    data = path.read_bytes()
    return LCP.from_bytes(data)


def save_hyber(lcp: LCP, path: Path) -> None:
    """Persist LCP struct to the HYBER save file."""
    path.write_bytes(lcp.to_bytes())


# ---------------------------------------------------------------------------
# Names file loader
# ---------------------------------------------------------------------------

def load_names(path: Path) -> list[str]:
    """
    Load the character name pool (DATA/NAMES).
    266 names × 10 bytes each, plain ASCII (no compression).
    addr: DATA/NAMES file, 266 × 10-byte entries
    """
    raw = path.read_bytes()
    names: list[str] = []
    for i in range(0, len(raw), 10):
        name = raw[i:i + 10].rstrip(b'\x00').decode('ascii', errors='replace')
        if name:
            names.append(name)
    return names


# ---------------------------------------------------------------------------
# Sound effect data loader
# ---------------------------------------------------------------------------

def load_sounds(path: Path) -> list[bytes]:
    """
    Load SOUNDS.LCP — 23 DoSound sequences.
    Returns a list of raw byte sequences, one per SOUND_EFFECT_ID.
    addr: soundeffect_load(), SOUNDS.LCP
    """
    raw = path.read_bytes()
    # Format: sequence of length-prefixed records (big-endian short length + data)
    sounds: list[bytes] = []
    offset = 0
    while offset < len(raw):
        if offset + 2 > len(raw):
            break
        length, = struct.unpack_from('>H', raw, offset)
        offset += 2
        sounds.append(raw[offset:offset + length])
        offset += length
    return sounds


# ---------------------------------------------------------------------------
# Song file loader
# ---------------------------------------------------------------------------

def load_song(path: Path) -> bytes:
    """Load a .SNG or .ORG music file as raw bytes for the MIDI sequencer."""
    return path.read_bytes()


# ---------------------------------------------------------------------------
# Master loader — populates GameState with all game assets
# ---------------------------------------------------------------------------

def load_all_assets(gs: GameState, data_dir: Optional[Path] = None) -> None:
    """
    Load every game data file and populate gs with sprites, images, and text.
    Call once at startup before entering the game loop.

    data_dir: path to the DATA/ directory (default: ./DATA relative to repo root)
    """
    if data_dir is None:
        data_dir = DEFAULT_DATA_DIR

    # -- Sprite definitions -------------------------------------------------
    sprites_path = data_dir / 'SPRITES'
    if sprites_path.exists():
        sprite_map = load_sprite_file(sprites_path, transparent=True)
        gs._sprite_images = {}  # logical_id → PIL Image (RGBA)
        for logical_id, (img, pix, mask, w, h) in sprite_map.items():
            if logical_id < 60:
                gs.sprite_def_image[logical_id]  = pix
                gs.sprite_def_mask[logical_id]   = mask
                gs.sprite_def_width[logical_id]  = w
                gs.sprite_def_height[logical_id] = h
                gs._sprite_images[logical_id]    = img

    # -- Object definitions (no transparency) --------------------------------
    objects_path = data_dir / 'OBJECTS'
    # Objects are rendered into the background, not via sprite pipeline

    # -- Save file -----------------------------------------------------------
    hyber_path = data_dir / 'HYBER'
    if hyber_path.exists():
        gs.lcp = load_hyber(hyber_path)
        gs.lcp_loaded = 1

    # -- Character name pool -------------------------------------------------
    names_path = data_dir / 'NAMES'
    if names_path.exists():
        gs._names = load_names(names_path)

    # -- Sound effects -------------------------------------------------------
    sounds_path = data_dir / 'SOUNDS.LCP'
    if sounds_path.exists():
        gs._sounds = load_sounds(sounds_path)

    # -- Body sprite sheet (active character variant) -----------------------
    # Body: sprite_lcp_flip(flipVertical=1) → dest bitplanes 0,1 → palette 0,1,2,3
    body_path = data_dir / 'BODY.LCP'
    if body_path.exists():
        gs._body_frames = load_lcp_sprite_sheet(body_path,
                                                 palette_indices=(0, 1, 2, 3))

    # -- Head sprite sheets (PE2–PE6, one per character variant) -------------
    # Head: sprite_lcp_flip(flipVertical=0) → dest bitplanes 1,2 → palette 0,2,4,6
    gs._head_frames: dict[int, list[Image.Image]] = {}
    for variant in range(2, 7):
        pe_path = data_dir / f'PE{variant}.LCP'
        if pe_path.exists():
            gs._head_frames[variant] = load_lcp_sprite_sheet(
                pe_path, palette_indices=(0, 2, 4, 6))

    # -- Playing cards -------------------------------------------------------
    cards_path = data_dir / 'CARDS'
    if cards_path.exists():
        gs._card_images = load_cards(cards_path)

    # -- Anagram word list ---------------------------------------------------
    words_path = data_dir / 'WORDS'
    if words_path.exists():
        gs._anagram_words = decompress_text_file(words_path).split()

    # -- Word puzzle templates -----------------------------------------------
    wordpz_path = data_dir / 'WORDPZ.TXT'
    if wordpz_path.exists():
        gs._word_puzzles = decompress_text_file(wordpz_path)

    # -- Letter templates ----------------------------------------------------
    letter_path = data_dir / 'LETTER.TXT'
    if letter_path.exists():
        gs._letter_templates = decompress_text_file(letter_path)

    # -- Background screens --------------------------------------------------
    house_scn = data_dir / 'HOUSE.SCN'
    if house_scn.exists():
        gs._house_background = load_screen(house_scn)

    title_scn = data_dir / 'TITLE.SCN'
    if title_scn.exists():
        gs._title_screen = load_screen(title_scn)

    # -- Music files ---------------------------------------------------------
    gs._songs: dict[str, bytes] = {}
    for stem in ['AISLEDAN', 'BALLAD', 'BEBOP', 'BOOGIE', 'BOSSA',
                 'CALYPSO', 'CANON', 'COUNTRY2', 'FIVEFOUR', 'MYSTERY', 'TANGO']:
        p = data_dir / f'{stem}.SNG'
        if p.exists():
            gs._songs[stem] = load_song(p)

    gs._organ_songs: dict[str, bytes] = {}
    for stem in ['FOLKSONG', 'MAPLE', 'PRELUDE', 'REQUIEM', 'STARSPAN']:
        p = data_dir / f'{stem}.ORG'
        if p.exists():
            gs._organ_songs[stem] = load_song(p)
