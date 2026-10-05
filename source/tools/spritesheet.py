#!/usr/bin/env python3
"""spritesheet.py -- render DATA/SPRITES and DATA/OBJECTS as labelled sheets.

Each graphic is drawn with the game's start-up palette (mainPalette in
dat_world.c) under its id and its name from include/enums.h, marked
"(unused)" where the define's comment says so:

  sprites  the 50 graphics in SPRITES, under the id spriteFileId assigns
           and their SPRITE_* name.  Colour 0 is transparent (makeMask
           leaves it out of the mask) and shows as a checkerboard.
  objects  the 56 graphics in OBJECTS, under their file position and
           their OBJ_* name.  drawObject blits them opaque, so colour 0
           is drawn as a colour.

Both files hold records of: height word, width word, then the image as
interleaved four-plane 16-pixel chunks (rows of ceil(width/16) chunks,
8 bytes each).  Only the first `width` columns are drawn.

usage: spritesheet.py [sprites|objects] [out.png]
       with no argument, writes docs/images/sprites.png and objects.png
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
IMAGES = os.path.join(ROOT, 'docs', 'images')

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


def palette():
    world = open(os.path.join(SRC, 'dat_world.c')).read()
    pal = []
    for v in re.findall(r'0x([0-9a-fA-F]{3})', c_array(world, 'mainPalette')):
        c = int(v, 16)
        pal.append(tuple(((c >> s) & 7) * 255 // 7 for s in (8, 4, 0)))
    return pal


def names(prefix):
    out = {}
    enums = open(os.path.join(SRC, 'include', 'enums.h')).read()
    for name, val, rest in re.findall(r'#define\s+(' + prefix + r'\w+)\s+(0x[0-9a-fA-F]+|\d+)\b(.*)', enums):
        out.setdefault(int(val, 0), []).append(name + (' (unused)' if 'unused' in rest else ''))
    return out


def read_records(path, count, pal, transparent):
    data = open(path, 'rb').read()
    out = []
    pos = 0
    for _ in range(count):
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
                    if v or not transparent:
                        img.putpixel((c * 16 + x, y), pal[v] + (255,))
        pos += chunks * h * 8
        out.append((w, h, img.crop((0, 0, w, h))))     # the blits stop at w
    return out


def render(entries, label_of, out):
    """entries: list of (id, w, h, image), sorted by id."""
    cell_w = max(max(img.width for _, _, _, img in entries) * SCALE + 2 * PAD, 250)
    cell_h = max(img.height for _, _, _, img in entries) * SCALE + 2 * PAD + LABEL_H
    rows = (len(entries) + COLS - 1) // COLS
    sheet = Image.new('RGB', (COLS * cell_w, rows * cell_h), (236, 236, 236))
    draw = ImageDraw.Draw(sheet)
    f_name, f_info = font(13), font(11)

    for n, (gid, w, h, img) in enumerate(entries):
        x0 = (n % COLS) * cell_w
        y0 = (n // COLS) * cell_h
        draw.rectangle((x0, y0, x0 + cell_w - 1, y0 + cell_h - 1), outline=(200, 200, 200))
        draw.text((x0 + PAD, y0 + 6), label_of(gid), fill=(20, 20, 20), font=f_name)
        draw.text((x0 + PAD, y0 + 21), '%d (0x%02x)  %dx%d' % (gid, gid, w, h),
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

    os.makedirs(os.path.dirname(out), exist_ok=True)
    sheet.save(out, optimize=True)
    print('wrote %s: %d graphics, %dx%d' % (out, len(entries), sheet.width, sheet.height))


def sprites(out):
    world = open(os.path.join(SRC, 'dat_world.c')).read()
    file_id = [int(x) for x in re.findall(r'\d+', c_array(world, 'spriteFileId'))]
    recs = read_records(os.path.join(ROOT, 'DATA', 'SPRITES'), len(file_id), palette(), True)
    entries = sorted((file_id[i], w, h, img) for i, (w, h, img) in enumerate(recs))
    nm = names('SPRITE_')
    render(entries, lambda gid: ', '.join(nm.get(gid, ['(unnamed)'])), out)


def objects(out):
    recs = read_records(os.path.join(ROOT, 'DATA', 'OBJECTS'), 56, palette(), False)
    entries = [(i, w, h, img) for i, (w, h, img) in enumerate(recs)]
    nm = names('OBJ_')
    render(entries, lambda gid: ', '.join(nm.get(gid, ['(unnamed)'])), out)


def main():
    kinds = {'sprites': sprites, 'objects': objects}
    if len(sys.argv) > 1:
        kind = sys.argv[1]
        out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(IMAGES, kind + '.png')
        kinds[kind](out)
    else:
        for kind, fn in kinds.items():
            fn(os.path.join(IMAGES, kind + '.png'))


if __name__ == '__main__':
    main()
