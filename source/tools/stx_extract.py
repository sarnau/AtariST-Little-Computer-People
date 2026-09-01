#!/usr/bin/env python3
"""stx_extract.py -- extract the game files from the Pasti disk image.

Parses "Little Computer People.stx" (Pasti/STX v3: RSY\\0 magic, one
track record per track with 16-byte sector descriptors; sector data
lives at DataOffset relative to the track data area, which follows
the descriptors and fuzzy mask -- same layout Hatari's floppy_stx.c
uses), reassembles the 80-track single-sided 9-sector FAT12 volume,
and extracts every file.

The disk carries the same DATA/ assets as the repo (all 32 files
byte-identical) plus the UNCRACKED LCP.PRG of the LARGER game
revision (123352 bytes; playable minigames + Timer-A MIDI + real
copy-protection) -- checked in as DATA/LCP_STX.PRG.  This is the
physical binary behind the "other Ghidra image"; DATA/LCP_ORG.PRG
(the cracked, smaller revision) remains the FAITHFUL build's
byte-identity reference.

Usage: python3 source/tools/stx_extract.py [image.stx] [outdir]
"""
import os, struct, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(
        os.path.abspath(__file__))))
STX = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        ROOT, 'Little Computer People.stx')
OUT = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
        ROOT, 'DATA', 'stx_extracted')


def read_sectors(path):
    d = open(path, 'rb').read()
    assert d[:4] == b'RSY\0', 'not a Pasti STX image'
    ntrk = d[10]
    sectors = {}
    p = 16
    for _ in range(ntrk):
        rec_size, fuzz_size, scount, flags = struct.unpack('<IIHH',
                                                           d[p:p + 12])
        tnum = d[p + 14]
        track, side = tnum & 0x7F, tnum >> 7
        if flags & 1:
            q = p + 16
            descs = []
            for _s in range(scount):
                off, = struct.unpack('<I', d[q:q + 4])
                R, N = d[q + 10], d[q + 11]
                fdc = d[q + 14]
                descs.append((off, R, N, fdc))
                q += 16
            data_area = p + 16 + 16 * scount + fuzz_size
            for off, R, N, fdc in descs:
                size = 128 << (N & 3)
                sectors[(track, side, R)] = \
                    d[data_area + off:data_area + off + size]
        p += rec_size
    return sectors


def logical_image(sectors):
    img = bytearray()
    tracks = 1 + max(k[0] for k in sectors)
    sides = 1 + max(k[1] for k in sectors)
    nsec = max(k[2] for k in sectors)
    for t in range(tracks):
        for h in range(sides):
            for s in range(1, nsec + 1):
                img += sectors.get((t, h, s), b'\0' * 512)
    return bytes(img)


def extract(img, outdir):
    bs = img[:512]
    bps, = struct.unpack('<H', bs[11:13])
    spc = bs[13]
    res, = struct.unpack('<H', bs[14:16])
    nfat = bs[16]
    ndirs, = struct.unpack('<H', bs[17:19])
    spf, = struct.unpack('<H', bs[22:24])
    fat = img[res * bps:(res + spf) * bps]
    root_off = (res + nfat * spf) * bps
    data_off = root_off + ndirs * 32

    def fatent(n):
        o = (n * 3) // 2
        v = fat[o] | (fat[o + 1] << 8)
        return (v >> 4) if n & 1 else (v & 0xFFF)

    def chain(clus, size=None):
        buf = b''
        c = clus
        while 2 <= c < 0xFF0:
            buf += img[data_off + (c - 2) * spc * bps:
                       data_off + (c - 1) * spc * bps]
            c = fatent(c)
        return buf if size is None else buf[:size]

    def walk(entries, outdir, depth=0):
        os.makedirs(outdir, exist_ok=True)
        for i in range(0, len(entries), 32):
            e = entries[i:i + 32]
            if len(e) < 32 or e[0] == 0:
                break
            if e[0] in (0xE5, 0x2E):
                continue
            name = e[:8].decode('latin1').rstrip()
            ext = e[8:11].decode('latin1').rstrip()
            fn = name + ('.' + ext if ext else '')
            attr = e[11]
            clus, = struct.unpack('<H', e[26:28])
            size, = struct.unpack('<I', e[28:32])
            if attr & 0x08:
                continue
            if attr & 0x10:
                walk(chain(clus), os.path.join(outdir, fn), depth + 1)
            else:
                open(os.path.join(outdir, fn), 'wb').write(chain(clus, size))
                print(f'{"  " * depth}{size:8d}  {fn}')

    walk(img[root_off:root_off + ndirs * 32], outdir)


def main():
    sectors = read_sectors(STX)
    print(f'{len(sectors)} sectors')
    extract(logical_image(sectors), OUT)
    print('->', OUT)


if __name__ == '__main__':
    main()
