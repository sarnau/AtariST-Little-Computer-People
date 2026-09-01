#!/usr/bin/env python3
"""bss_remap.py -- re-lay the FAITHFUL build's BSS to the ROM's layout.

The 1985 linker that produced LCP_ORG.PRG allocated `.comm` blocks in
an order none of the surviving toolchain linkers reproduce (native
lo68/link68 allocate hash-grouped, the 1990 ALN.PRG allocates
alphabetically; the ROM's order matches neither, nor first-mention
order).  TEXT, DATA and the relocation stream are byte-identical out
of lo68 -- the only bytes that differ are relocated longwords whose
targets sit in BSS, because the two linkers packed the same commons at
different offsets.

This tool derives the address translation {port BSS addr -> ROM BSS
addr} from the relocation stream itself: TEXT/DATA/RELOC being
byte-identical means site N in the port and site N in the ROM are the
same operand of the same instruction, so pairing the two longwords at
every site yields the mapping.  The substance is the consistency
check: every port address must map to exactly ONE ROM address across
all sites that reference it, which proves the port's reference
structure (which symbol each site addresses, at which offset) matches
the ROM's exactly.  Only then is the mapping applied and the header
BSS size set to the ROM's.

Usage: python3 source/tools/bss_remap.py [port.prg] [orig.prg]
Rewrites port.prg in place.  Exits non-zero on any inconsistency.
"""
import os, struct, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PORT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        ROOT, 'build', 'alcyon', 'LCP.PRG')
ORIG = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
        ROOT, '..', 'DATA', 'LCP_ORG.PRG')


def sites(reloc):
    if not reloc:
        return
    pos = struct.unpack('>I', reloc[:4])[0]
    yield pos
    i = 4
    while i < len(reloc):
        c = reloc[i]; i += 1
        if c == 0:
            break
        if c == 1:
            pos += 254
        else:
            pos += c
            yield pos


def main():
    pd = bytearray(open(PORT, 'rb').read())
    od = open(ORIG, 'rb').read()
    _, pt, pdd, pb, ps = struct.unpack('>HIIII', pd[:18])
    _, ot, odd, ob, osz = struct.unpack('>HIIII', od[:18])
    if (pt, pdd) != (ot, odd):
        sys.exit('text/data sizes differ -- fix those before remapping BSS')
    preloc = bytes(pd[0x1C + pt + pdd + ps:])
    oreloc = od[0x1C + ot + odd + osz:]
    if preloc != oreloc:
        sys.exit('relocation streams differ -- BSS remap needs them identical')

    bss_lo = pt + pdd
    mapping = {}
    for pos in sites(preloc):
        pv = struct.unpack('>I', pd[0x1C + pos:0x1C + pos + 4])[0]
        ov = struct.unpack('>I', od[0x1C + pos:0x1C + pos + 4])[0]
        if pv < bss_lo and ov < bss_lo:
            if pv != ov:
                sys.exit(f'text/data-target mismatch at site +{pos:#x}: '
                         f'port {pv:#x} orig {ov:#x}')
            continue
        if (pv < bss_lo) != (ov < bss_lo):
            sys.exit(f'segment mismatch at site +{pos:#x}: '
                     f'port {pv:#x} orig {ov:#x}')
        if pv in mapping and mapping[pv] != ov:
            sys.exit(f'inconsistent mapping for port {pv:#x}: '
                     f'{mapping[pv]:#x} vs {ov:#x} at site +{pos:#x}')
        mapping[pv] = ov
    # a consistent map may still fold two port cells onto one ROM cell
    rev = {}
    for k, v in mapping.items():
        if v in rev:
            sys.exit(f'two port addresses ({rev[v]:#x}, {k:#x}) map to '
                     f'ROM {v:#x}')
        rev[v] = k

    n = 0
    for pos in sites(preloc):
        pv = struct.unpack('>I', pd[0x1C + pos:0x1C + pos + 4])[0]
        if pv in mapping:
            pd[0x1C + pos:0x1C + pos + 4] = struct.pack('>I', mapping[pv])
            n += 1
    pd[10:14] = struct.pack('>I', ob)          # header BSS size
    open(PORT, 'wb').write(pd)
    print(f'bss_remap: {len(mapping)} BSS addresses, {n} sites rewritten, '
          f'bss size {pb} -> {ob}')


if __name__ == '__main__':
    main()
