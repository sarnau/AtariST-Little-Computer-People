#!/usr/bin/env python3
"""prg_diff.py -- whole-file comparison of the build vs DATA/LCP_STX.PRG.

The end-state check for the byte-identical goal: compares header
section sizes, then text, data, and relocation streams byte-for-byte
(NO wildcards -- addresses must be identical too), reporting the first
divergence in each and a running tally.

Usage: python3 source/tools/prg_diff.py [port.prg] [orig.prg]
"""
import os, struct, sys

MASK = '--mask' in sys.argv
sys.argv = [a for a in sys.argv if a != '--mask']

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PORT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        ROOT, 'build', 'alcyon', 'LCP.PRG')
ORIG = sys.argv[2] if len(sys.argv) > 2 else \
       os.environ.get('LCP_REF') or os.path.join(
        ROOT, '..', 'DATA', 'LCP_STX.PRG')


def load(path):
    d = open(path, 'rb').read()
    magic, t, dd, b, s = struct.unpack('>HIIII', d[:18])
    assert magic == 0x601A, path
    text = d[0x1C:0x1C + t]
    data = d[0x1C + t:0x1C + t + dd]
    reloc = d[0x1C + t + dd + s:]
    if MASK and reloc:
        first = struct.unpack('>I', reloc[:4])[0]
        blob = bytearray(text + data)
        if first:
            pos, i = first, 4
            blob[pos:pos + 4] = b'\0\0\0\0'
            while i < len(reloc):
                c = reloc[i]; i += 1
                if c == 0:
                    break
                if c == 1:
                    pos += 254
                else:
                    pos += c
                    blob[pos:pos + 4] = b'\0\0\0\0'
        text, data = bytes(blob[:t]), bytes(blob[t:])
    return t, dd, b, text, data, reloc


def first_diff(a, b):
    n = min(len(a), len(b))
    for i in range(n):
        if a[i] != b[i]:
            return i
    return -1 if len(a) == len(b) else n


def tally(a, b):
    n = min(len(a), len(b))
    return sum(1 for i in range(n) if a[i] != b[i]) + abs(len(a) - len(b))


def ctx(buf, pos, n=12):
    return ' '.join(f'{x:02x}' for x in buf[max(0, pos - 4):pos + n])


def main():
    pt, pd, pb, ptext, pdata, prel = load(PORT)
    ot, od, ob, otext, odata, orel = load(ORIG)
    print(f'          {"port":>10} {"orig":>10}')
    print(f'  text    {pt:>10} {ot:>10}  {"==" if pt == ot else f"delta {pt-ot:+d}"}')
    print(f'  data    {pd:>10} {od:>10}  {"==" if pd == od else f"delta {pd-od:+d}"}')
    print(f'  bss     {pb:>10} {ob:>10}  {"==" if pb == ob else f"delta {pb-ob:+d}"}')
    for name, a, b in (('TEXT', ptext, otext), ('DATA', pdata, odata),
                       ('RELOC', prel, orel)):
        i = first_diff(a, b)
        if i < 0:
            print(f'  {name}: IDENTICAL ({len(a)} bytes)')
        else:
            print(f'  {name}: first diff at +{i:#x}; '
                  f'{tally(a, b)} differing/extra bytes')
            print(f'    port.. {ctx(a, i)}')
            print(f'    orig.. {ctx(b, i)}')
    if (pt, pd, pb) == (ot, od, ob) and ptext == otext and \
            pdata == odata and prel == orel:
        print('\n*** BYTE-IDENTICAL ***')


if __name__ == '__main__':
    main()
