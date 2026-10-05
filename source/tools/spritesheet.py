#!/usr/bin/env python3
"""spritesheet.py -- render DATA/SPRITES as a labelled sprite sheet.

Each of the 50 graphics in SPRITES is drawn with the game's start-up
palette (mainPalette in dat_world.c), under the id spriteFileId assigns
it and its SPRITE_* name from include/enums.h.  Transparent pixels
(colour 0, which makeMask leaves out of the mask) show as a checkerboard.

SPRITES record: height word, width word, then the image as interleaved
four-plane 16-pixel chunks (rows of ceil(width/16) chunks, 8 bytes each).

usage: spritesheet.py [out.png]      default: docs/images/sprites.png
Needs Pillow.
"""
import os
import re
import struct
import sys

from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.dirname(HERE)
ROOT = os.path.dirname(SRC)
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'docs', 'images', 'sprites.png')

SCALE = 3
COLS = 5
PAD = 10
LABEL_H = 34


def c_array(text, name):
    m = re.search(name + r'\[[^\]]*\]\s*=\s*\{(.*?)\};', text, re.S)
    return re.sub(r'/\*.*?\*/', '', m.group(1), flags=re.S)


def font(size):
    for path in ('/System/Library/Fonts/Menlo.ttc',
                 '/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf'):
        if os.path.exists(path):
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def main():
    world = open(os.path.join(SRC, 'dat_world.c')).read()
    palette = []
    for v in re.findall(r'0x([0-9a-fA-F]{3})', c_array(world, 'mainPalette')):
        c = int(v, 16)
        palette.append(tuple(((c >> s) & 7) * 255 // 7 for s in (8, 4, 0)))
    file_id = [int(x) for x in re.findall(r'\d+', c_array(world, 'spriteFileId'))]

    names = {}
    for name, val in re.findall(r'#define\s+(SPRITE_\w+)\s+(0x[0-9a-fA-F]+)',
                                open(os.path.join(SRC, 'include', 'enums.h')).read()):
        names.setdefault(int(val, 16), []).append(name)

    data = open(os.path.join(ROOT, 'DATA', 'SPRITES'), 'rb').read()
    sprites = []
    pos = 0
    for i in range(len(file_id)):
        h, w = struct.unpack('>hh', data[pos:pos + 4])
        pos += 4
        chunks = (w + 15) // 16
        img = Image.new('RGBA', (chunks * 16, h), (0, 0, 0, 0))
        for y in range(h):
            for c in range(chunks):
                off = pos + (y * chunks + c) * 8
                planes = struct.unpack('>4H', data[off:off + 8])
                for x in range(16):
                    bit = 15 - x
                    v = sum(((planes[p] >> bit) & 1) << p for p in range(4))
                    if v:
                        img.putpixel((c * 16 + x, y), palette[v] + (255,))
        pos += chunks * h * 8
        sprites.append((file_id[i], w, h, img))
    sprites.sort()

    cell_w = max(img.width for _, _, _, img in sprites) * SCALE + 2 * PAD
    cell_w = max(cell_w, 230)
    cell_h = max(img.height for _, _, _, img in sprites) * SCALE + 2 * PAD + LABEL_H
    rows = (len(sprites) + COLS - 1) // COLS
    sheet = Image.new('RGB', (COLS * cell_w, rows * cell_h), (236, 236, 236))
    draw = ImageDraw.Draw(sheet)
    f_name, f_info = font(13), font(11)

    for n, (sid, w, h, img) in enumerate(sprites):
        x0 = (n % COLS) * cell_w
        y0 = (n // COLS) * cell_h
        draw.rectangle((x0, y0, x0 + cell_w - 1, y0 + cell_h - 1), outline=(200, 200, 200))
        label = ', '.join(names.get(sid, ['(unnamed)']))
        draw.text((x0 + PAD, y0 + 6), label, fill=(20, 20, 20), font=f_name)
        draw.text((x0 + PAD, y0 + 21), '%d (0x%02x)  %dx%d' % (sid, sid, w, h),
                  fill=(110, 110, 110), font=f_info)
        big = img.resize((img.width * SCALE, img.height * SCALE), Image.NEAREST)
        bx, by = x0 + PAD, y0 + LABEL_H + PAD
        for cy in range(0, big.height, 6):          # checkerboard = transparent
            for cx in range(0, big.width, 6):
                shade = (150, 150, 150) if (cx // 6 + cy // 6) % 2 else (175, 175, 175)
                draw.rectangle((bx + cx, by + cy,
                                bx + min(cx + 5, big.width - 1), by + min(cy + 5, big.height - 1)),
                               fill=shade)
        sheet.paste(big, (bx, by), big)

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    sheet.save(OUT, optimize=True)
    print('wrote %s: %d sprites, %dx%d' % (OUT, len(sprites), sheet.width, sheet.height))


if __name__ == '__main__':
    main()
