# LCP.PRG — Image & Graphics Format Reference

## Overview

Little Computer People uses the Atari ST's low-resolution mode (320×200,
16 colors, 4 bitplanes). All image data uses the standard ST interleaved
bitplane format. The game employs three distinct image systems: compressed
full-screen backgrounds, variable-size sprite/object bitmaps, and fixed-size
animation frame sets.

### Pixel Format: Atari ST 4-Plane Interleaved

All image data in LCP uses the Atari ST's native 4-bitplane interleaved
format. Pixels are stored in 16-pixel words, with 4 consecutive 16-bit
values holding one bitplane each:

```
For each 16-pixel horizontal group:
  Word 0: Bitplane 0 (bit 0 of each pixel's color index)
  Word 1: Bitplane 1 (bit 1 of each pixel's color index)
  Word 2: Bitplane 2 (bit 2 of each pixel's color index)
  Word 3: Bitplane 3 (bit 3 of each pixel's color index)

  Total: 8 bytes per 16-pixel group
```

For a pixel at position P (0–15) within a word group, the 4-bit color
index is assembled as:

```
color = ((plane0 >> (15-P)) & 1) << 0
      | ((plane1 >> (15-P)) & 1) << 1
      | ((plane2 >> (15-P)) & 1) << 2
      | ((plane3 >> (15-P)) & 1) << 3
```

Images wider than 16 pixels use multiple word groups per scanline.
The number of word groups per line is `ceil(width / 16)`, and each
group is 8 bytes, so bytes per scanline = `ceil(width / 16) × 8`.

### Color Palette

The game uses a fixed 16-color palette stored in `main_colorpalette[]`
and loaded via XBIOS `Setpalette`. Atari ST color registers encode RGB
as 3 octal digits in a 16-bit word: bits 8–10 = Red, bits 4–6 = Green,
bits 0–2 = Blue, each 0–7.

| Index | ST Value | R | G | B | RGB (approx.) | Usage |
|---|---|---|---|---|---|---|
| 0 | 0x0000 | 0 | 0 | 0 | #000000 | Black / transparent |
| 1 | 0x0442 | 4 | 4 | 2 | #808040 | Dark olive |
| 2 | 0x0265 | 2 | 6 | 5 | #40C0A0 | Teal |
| 3 | 0x0754 | 7 | 5 | 4 | #E0A080 | Peach (skin) |
| 4 | 0x0310 | 3 | 1 | 0 | #602000 | Dark brown |
| 5 | 0x0040 | 0 | 4 | 0 | #008000 | Green |
| 6 | 0x0754 | 7 | 5 | 4 | #E0A080 | Peach / sick green (dynamic) |
| 7 | 0x0760 | 7 | 6 | 0 | #E0C000 | Yellow |
| 8 | 0x0247 | 2 | 4 | 7 | #4080E0 | Blue |
| 9 | 0x0631 | 6 | 3 | 1 | #C06020 | Orange-brown |
| 10 | 0x0700 | 7 | 0 | 0 | #E00000 | Red |
| 11 | 0x0333 | 3 | 3 | 3 | #606060 | Dark gray |
| 12 | 0x0555 | 5 | 5 | 5 | #A0A0A0 | Medium gray |
| 13 | 0x0007 | 0 | 0 | 7 | #0000E0 | Blue |
| 14 | 0x0777 | 7 | 7 | 7 | #E0E0E0 | White |
| 15 | 0x0410 | 4 | 1 | 0 | #802000 | Brown |

Color index 0 (black) doubles as the transparent color for sprite
overlays. Color index 6 is dynamically swapped between peach (0x0754,
`ST_PEACH`) and sick green (`ST_SICK_GREEN`) by `lcp_update_palette_colors()`
based on the LCP's `sickness_level`.

---

## File Formats

### 1. Compressed Screen Images (.SCN)

Used for: `HOUSE.SCN` (house background), `TITLE.SCN` (title screen).

These are full 320×200 screen images compressed using a nibble-packed
dictionary scheme. Decompressed output is 32,000 bytes (standard ST
low-res screen: 200 lines × 20 word-groups × 8 bytes).

#### File Structure

```
Offset  Size  Content
0x00      2   File size (big-endian short, equals total file length)
0x02     30   Dictionary: 15 big-endian 16-bit words (indices 0–14)
0x20    var   Compressed nibble stream
```

#### Compression Algorithm

The compressed data is a stream of 4-bit nibbles, read alternately
from the high and low nibble of each byte (high nibble first).

For each nibble:
- **Nibble 0x0–0xE**: Output `dictionary[nibble]` (a 16-bit word from
  the 15-entry header table). This is the common case — most screen
  words match one of 15 frequently-occurring values.
- **Nibble 0xF** (escape): Read the next 4 nibbles and assemble them
  into a literal 16-bit word value (MSB first). This handles words
  not in the dictionary.

Each decompressed unit is one 16-bit word. The full screen requires
16,000 words (32,000 bytes).

#### Compression Ratios

| File | Compressed | Decompressed | Ratio |
|---|---|---|---|
| HOUSE.SCN | 20,048 bytes | 32,000 bytes | 62.7% |
| TITLE.SCN | 12,304 bytes | 32,000 bytes | 38.5% |

The title screen compresses better because it has more uniform
regions (solid color areas).

---

### 2. Sprite File (SPRITES)

Used for: overlay sprites rendered on top of the house background with
transparency. Sprites are composited using a mask-based technique
(AND-mask then XOR-image) via the VDI `vro_cpyfm` function in
`sprite_draw()`.

#### File Structure

The file contains 50 variable-size sprite images stored sequentially
with no global header. Each entry:

```
Offset  Size   Content
0x00      2    Height in pixels (big-endian short)
0x02      2    Width in pixels (big-endian short)
0x04    var    4-plane interleaved bitmap data
               Size = ceil(width/16) × 8 × height bytes
```

Total file size: 12,872 bytes (50 entries, fully consumed).

The game generates transparency masks at runtime from the image data:
any pixel with color index 0 (black) is treated as transparent.

#### Sprite Index Table

Sprites are not stored in sequential ID order. The game uses
`spritedata_index_table[]` to map file position to logical sprite ID.
File entry N corresponds to sprite ID `spritedata_index_table[N]`:

```
File entry:  0  1  2  3  4  5  6  7  8  9 10 11 12 ...
Sprite ID:  0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 ...

File entry: 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49
Sprite ID:  09 2D 2E 2F 30 31 03 04 32 07 06 33 34 35 36 08 37
```

#### Sprite Catalog by Dimension

| Dimensions | Count | Description |
|---|---|---|
| 48×13 | 7 | Large horizontal sprites (LCP walking frames) |
| 26×15 | 12 | Medium sprites (dog animation frames) |
| 18×36 | 6 | Tall sprites (LCP standing poses) |
| 18×37 | 3 | Tall sprites (LCP action poses) |
| 16×37 | 3 | Narrow tall sprites (door sprites) |
| 24×7 | 4 | Wide short sprites (speech bubbles?) |
| 48×16 | 1 | Extra-large sprite |
| 12×9 | 3 | Small sprites |
| Other | 11 | Various small UI/detail sprites |

#### Sprite Rendering Pipeline

The game maintains 8 hardware sprite slots. For each active slot,
`sprite_draw()` composites the sprite onto the screen backbuffer:

1. **NOTS_AND_D**: AND the inverted mask with the destination
   (punches a transparent hole in the background)
2. **S_XOR_D**: XOR the sprite image into the cleared area
   (paints the sprite pixels)

The dog sprite supports horizontal flipping via `sprite_flip_horizontal()`,
which reverses word order per scanline and uses a 256-entry
`revert_table[]` for bit-level pixel reversal within each word.

---

### 3. Object File (OBJECTS)

Used for: static background elements drawn directly into the screen
buffer — doors, furniture states, food items, fire animation frames,
phone, alarm clock, etc. Unlike sprites, objects have no transparency
and are blitted using VDI `S_ONLY` mode (direct overwrite) via
`object_draw()`.

#### File Structure

Same format as SPRITES: sequential entries with height, width, and
raw 4-plane bitmap data. No global header.

```
Offset  Size   Content
0x00      2    Height in pixels (big-endian short)
0x02      2    Width in pixels (big-endian short)
0x04    var    4-plane interleaved bitmap data
```

Total file size: 13,504 bytes (56 entries, fully consumed).

Objects are loaded into `objects_file` by `load_objects()` (reads
"objects" file, up to 14,000 bytes). Individual objects are accessed
by `object_draw()` via the `object_tab_mfdb[]`, `object_tab_width[]`,
and `object_tab_height[]` lookup tables.

#### Object Catalog by Dimension

| Dimensions | Count | Likely Content |
|---|---|---|
| 18×36 | 6 | Door states (open/closed/ajar) |
| 18×37 | 3 | Tall door/furniture states |
| 24×17 | 5 | Medium furniture objects |
| 20×34 | 3 | Tall furniture (closet/cabinet states) |
| 21×27 | 3 | Furniture (dresser states) |
| 26×12 | 4 | Wide objects (shelf/counter states) |
| 16×37 | 3 | Narrow door panels |
| 32×7 | 4 | Wide flat objects |
| 16×10–11 | 8 | Small objects (food, items) |
| 16×8 | 5 | Small objects |
| 16×18 | 3 | Medium-small objects |
| 16×3 | 4 | Tiny objects (indicators, handles) |
| 8×3 | 3 | Very small objects (status indicators) |
| Other | 2 | Miscellaneous small elements |

Objects typically come in groups of 3 (open/closed/intermediate state)
for animated furniture interactions.

---

### 4. Playing Card File (CARDS)

Used for: the card game (solitaire) that the LCP plays at the table.

#### File Structure

No header. Raw sequential 4-plane bitmap data for 53 cards:

```
Card size: 16×24 pixels × 4 planes = 192 bytes per card
File size: 10,176 bytes = 53 × 192 bytes (no remainder)
```

The 53 cards likely represent a standard 52-card deck plus one card
back design (shown for face-down cards).

---

### 5. Person/Body Sprite Files (PE*.LCP, BODY.LCP)

Used for: LCP character body animation frames. Each PE file contains
a complete set of body poses for one character appearance variant.
The game ships with 5 PE files (PE2–PE6), each representing a
different character appearance (clothing, hair color, skin tone).

#### File Structure

```
Offset  Size    Content
0x00      2     Frame count (big-endian short)
0x02      2     Data size in bytes (big-endian short)
0x04    var     Frame data: count × 168 bytes
```

Each frame is a fixed 16×21 pixel sprite in 4-plane format:

```
Frame size: 16 pixels wide = 1 word group
            21 pixels tall
            1 × 8 × 21 = 168 bytes per frame
```

#### PE File Specifications

| File | Frames | Data Size | Total | Description |
|---|---|---|---|---|
| PE2.LCP | 66 | 11,088 | 11,092 | Character variant 2 |
| PE3.LCP | 66 | 11,088 | 11,092 | Character variant 3 |
| PE4.LCP | 66 | 11,088 | 11,092 | Character variant 4 |
| PE5.LCP | 66 | 11,088 | 11,092 | Character variant 5 |
| PE6.LCP | 66 | 11,088 | 11,092 | Character variant 6 |

All PE files have identical structure (66 frames, 11,088 bytes of image
data) but differ in pixel content — each file has 1,000–1,600 bytes of
difference from any other, representing the visual variations between
character appearances while sharing the same pose/animation structure.

The 66 frames cover the full set of LCP body poses: walking (left/right,
8 frames each), standing, sitting, sleeping, eating, typing, exercising,
carrying objects, and various interaction poses.

BODY.LCP uses the same format and contains the base character body
sprites: 98 frames × 168 bytes = 16,464 bytes of image data (file
size 16,468 bytes including the 4-byte header).  Each PE*.LCP file
is layered on top of the corresponding BODY.LCP frames at runtime
to produce the finished character sprite.

---

## Rendering Architecture

### Double-Buffered Compositing

The game uses double-buffered rendering at approximately 8 Hz,
managed by `screen_render_8hz()`:

1. **Background copy**: The decompressed house background is copied
   from an offscreen buffer to the compositing buffer via `blkcopy32()`
   (32-byte aligned block copy, 1000 longwords = 32,000 bytes).

2. **Object rendering**: Static background objects (doors, furniture,
   food) are drawn directly into the compositing buffer using
   `object_draw()` with VDI `S_ONLY` mode (no transparency).

3. **Sprite compositing**: Up to 8 overlay sprites are composited
   using the mask-based AND/XOR technique in `sprite_draw()`.
   The LCP character and dog occupy sprite slots; remaining slots
   handle speech bubbles and other overlays.

4. **Page flip**: `XBIOS Vsync` + `XBIOS Setscreen` swaps the
   display to the newly composited buffer. The two buffers alternate
   each frame.

### Sprite Slot Allocation

The game maintains 8 sprite rendering slots (indices 0–7). Slot
assignment is managed by `sprite_update_slots()` using
`sprite_layer_flags[]` to determine draw order:

- Slot 0: Dog sprite (behind player)
- Slot 7: Dog sprite (in front of player, based on Y-depth)
- Slots 1–6: LCP body parts, speech bubbles, held objects

Each slot stores: image pointer, mask pointer, width, height, X
position, and Y position. A pending/active double-buffer system
prevents tearing during slot updates.

---

## Summary Table

| File | Format | Entries | Dimensions | Compression | Purpose |
|---|---|---|---|---|---|
| HOUSE.SCN | Nibble-dict compressed | 1 | 320×200 | Yes (62.7%) | House background |
| TITLE.SCN | Nibble-dict compressed | 1 | 320×200 | Yes (38.5%) | Title screen |
| SPRITES | Raw sequential + index | 50 | Variable | No | Overlay sprites (transparent) |
| OBJECTS | Raw sequential | 56 | Variable | No | Background objects (opaque) |
| CARDS | Raw sequential | 53 | 16×24 | No | Playing cards |
| PE2–PE6.LCP | Header + fixed frames | 66 each | 16×21 | No | Character body variants |
| BODY.LCP | Header + fixed frames | 98 | 16×21 | No | Base character body |
