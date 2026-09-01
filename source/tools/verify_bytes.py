#!/usr/bin/env python3
"""verify_bytes.py -- prove each ported function byte-faithful vs LCP_ORG.PRG.

For every text symbol in the port (lcp_sym.68k, linked without -s), take
its code bytes from the built LCP.PRG, wildcard every layout-dependent
byte -- longwords the relocation table marks (absolute addresses) and
PC-relative call/branch displacements (bsr.w/bsr.s/bra.w, jsr/jmp
d16(pc)) -- and search the ORIGINAL game's text segment for the
resulting pattern.

  MATCH      -> the compiled function is byte-identical to the ROM
                (modulo layout); we also recover its original address.
  DIVERGENT  -> no site in the original matches; the recreated C does
                not compile to the ROM's code.  A prefix probe reports
                the best candidate site, the first mismatching byte,
                and hex context on both sides.

Needs neither Ghidra nor symbols in the original.  Library code
(gemlib/vdibind/aesbind) is compared exactly like game code -- note the
original shipped with an OLDER VDI binding library than DK DISK_2's
VDIBIND (per-parameter push helper instead of array writes), so the
v*/vs* group diverging is a library-revision issue, not game source.

Usage:
  python3 source/tools/verify_bytes.py            # summary + divergent list
  python3 source/tools/verify_bytes.py -v         # per-function lines
  python3 source/tools/verify_bytes.py sp_updb …  # only these (port names)

Prereq: build + link, then create the symbol link (same object list as
alcyon_link.sh but without -s):
  lo68 -r -o lcp_sym.68k gemstart.o main.o <objs> vdibind.a … libf
"""
import os, re, struct, sys

ROOT   = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD  = os.path.join(ROOT, 'build', 'alcyon')
PORT   = os.path.join(BUILD, 'LCP.PRG')
SYM68K = os.path.join(BUILD, 'lcp_sym.68k')
ORIG   = os.path.join(ROOT, '..', 'DATA', 'LCP_ORG.PRG')

MIN_UNIQUE = 10   # fixed bytes needed before we trust a match
PROBE      = 24   # prefix length used to hunt a candidate site


def read_prg(path):
    d = open(path, 'rb').read()
    magic, t, dd, b, s = struct.unpack('>HIIII', d[:18])
    assert magic == 0x601A, path
    text = d[0x1C:0x1C + t]
    reloc_off = 0x1C + t + dd + s
    relocs = set()
    if reloc_off < len(d):
        first = struct.unpack('>I', d[reloc_off:reloc_off + 4])[0]
        if first != 0:
            pos, i = first, reloc_off + 4
            relocs.add(pos)
            while i < len(d):
                c = d[i]; i += 1
                if c == 0:
                    break
                if c == 1:
                    pos += 254
                else:
                    pos += c
                    relocs.add(pos)
    return text, t, relocs


def read_syms(path, tsize):
    d = open(path, 'rb').read()
    magic, t, dd, b, s = struct.unpack('>HIIII', d[:18])
    assert magic == 0x601A and t == tsize, 'symbol link differs from PRG'
    off, end = 0x1C + t + dd, 0x1C + t + dd + s
    syms = {}
    while off < end:
        name = d[off:off + 8].rstrip(b'\0').decode('ascii', 'replace')
        typ, val = struct.unpack('>HI', d[off + 8:off + 14])
        off += 14
        if typ & 0x0200 and val < t:          # defined, text segment
            syms.setdefault(val, name)
    return sorted(syms.items())               # [(text_offset, name)]


def tokens(code, base, relocs):
    """Yield (offset_in_code, width, fixed?) walking the code word-wise.

    Wildcards (fixed=False): relocated longwords, and the displacement
    of bsr.w/bra.w/jsr d16(pc)/jmp d16(pc) and bsr.s -- those change
    when OTHER functions move, without any source difference.
    """
    i = 0
    while i < len(code):
        if base + i in relocs:
            yield i, 4, False
            i += 4
            continue
        w = code[i:i + 2]
        if len(w) == 2 and i + 3 < len(code) and \
                w in (b'\x61\x00', b'\x60\x00', b'\x4e\xba', b'\x4e\xfa'):
            yield i, 2, True
            yield i + 2, 2, False
            i += 4
        elif len(w) == 2 and w[0] == 0x61 and w[1] != 0:
            yield i, 1, True
            yield i + 1, 1, False
            i += 2
        else:
            yield i, len(w), True
            i += len(w)


def pattern(code, base, relocs):
    out, fixed = [], 0
    for off, width, fix in tokens(code, base, relocs):
        if fix:
            out.append(re.escape(code[off:off + width]))
            fixed += width
        else:
            out.append(b'.{%d}' % width)
    return b''.join(out), fixed


def first_mismatch(code, base, relocs, otext, oo):
    """Pattern-aware compare of `code` against otext[oo:].

    Returns (first_any, first_hard): a conditional-branch displacement
    that differs is a SOFT mismatch (a downstream size change echoes
    into every bcc.w/bcc.s over it); the first non-branch difference is
    the HARD one that names the real divergence.  Either is -1 if none.
    """
    first_any = first_hard = -1
    for off, width, fix in tokens(code, base, relocs):
        if not fix:
            continue
        if oo + off + width > len(otext):
            return (off, off) if first_any < 0 else (first_any, off)
        if code[off:off + width] == otext[oo + off:oo + off + width]:
            continue
        if first_any < 0:
            first_any = off
        # soft: bcc.w displacement word (preceded by 6xxx 00 opcode) or
        # differing low byte of a bcc.s (opcode byte 0x6x, same opcode)
        prev = code[off - 2:off]
        if width == 2 and len(prev) == 2 and (prev[0] & 0xF0) == 0x60:
            continue
        if width == 2 and (code[off] & 0xF0) == 0x60 and \
                code[off] == otext[oo + off] and width == 2:
            continue
        first_hard = off
        break
    return first_any, first_hard


def hexctx(buf, pos, lo, hi):
    return ' '.join(f'{buf[k]:02x}' for k in range(max(0, pos + lo),
                                                   min(len(buf), pos + hi)))


def main():
    verbose = '-v' in sys.argv
    want = {a for a in sys.argv[1:] if not a.startswith('-')}

    ptext, ptsize, prelocs = read_prg(PORT)
    otext, otsize, orelocs = read_prg(ORIG)
    syms = read_syms(SYM68K, ptsize)

    bounds = syms + [(ptsize, '<end>')]
    matched, divergent, skipped = [], [], []
    claimed = bytearray(otsize)

    for k, (off, name) in enumerate(syms):
        if want and name.lstrip('_') not in want and name not in want:
            continue
        size = bounds[k + 1][0] - off
        if size <= 0:
            continue
        code = ptext[off:off + size]
        pat, fixed = pattern(code, off, prelocs)
        if fixed < MIN_UNIQUE:
            skipped.append((name, off, size))
            continue
        hits = [m.start() for m in re.finditer(pat, otext, re.DOTALL)]
        if hits:
            oo = hits[0]
            claimed[oo:oo + size] = b'\1' * size
            matched.append((name, off, size, oo, len(hits)))
            if verbose:
                extra = '' if len(hits) == 1 else f'  ({len(hits)} sites)'
                print(f'MATCH     {name:<10} port=0x{off:05x} '
                      f'orig=0x{oo:05x} len={size}{extra}')
        else:
            ppat, pfx = pattern(code[:min(PROBE, size)], off, prelocs)
            cand = [m.start() for m in re.finditer(ppat, otext, re.DOTALL)] \
                   if pfx >= 6 else []
            where = ''
            if cand:
                oo = cand[0]
                j_any, j = first_mismatch(code, off, prelocs, otext, oo)
                if j < 0:
                    j = j_any
                soft = '' if j == j_any else (f' (soft branch-disp diff '
                                              f'from +0x{j_any:x})')
                where = (f'\n            candidate orig=0x{oo:05x}, first '
                         f'hard mismatch at +0x{j:x}{soft}:'
                         f'\n            port.. {hexctx(ptext, off + j, -6, 10)}'
                         f'\n            orig.. {hexctx(otext, oo + j, -6, 10)}')
            divergent.append((name, off, size))
            print(f'DIVERGENT {name:<10} port=0x{off:05x} len={size}{where}')

    cov = sum(1 for b in claimed if b) if matched else 0
    print(f'\n{len(matched)} matched, {len(divergent)} divergent, '
          f'{len(skipped)} too-small-to-verify; '
          f'original text coverage {cov}/{otsize} bytes '
          f'({100.0 * cov / otsize:.1f}%)')
    if skipped and verbose:
        for name, off, size in skipped:
            print(f'SKIP      {name:<10} port=0x{off:05x} len={size} '
                  f'(mostly relocations)')
    return 1 if divergent else 0


if __name__ == '__main__':
    sys.exit(main())
