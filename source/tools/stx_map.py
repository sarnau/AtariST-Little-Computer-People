#!/usr/bin/env python3
"""stx_map.py -- reconstruct LCP_STX's symbol/address map from matches.

For the byte-identical build target we must reproduce LCP_STX's exact
layout.  Every byte-matched function gives us two kinds of evidence:

  * its own text address in the reference (where it matched), and
  * at every relocation site, a pairing between the port operand
    (port symbol + offset) and the reference operand (an address).

Aggregating those pairs over all matched functions yields the ROM
address of every referenced global -- i.e. the target DATA/BSS layout
-- and the per-object function ordering yields the link order.

Usage:
  python3 source/tools/stx_map.py            # summary + conflicts
  python3 source/tools/stx_map.py --data     # symbol -> address table
  python3 source/tools/stx_map.py --objects  # link-order report
"""
import glob, os, re, struct, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from verify_bytes import (read_prg, pattern, tokens, PORT, SYM68K, ORIG,
                          MIN_UNIQUE, BUILD)


def read_all_syms(path):
    """All defined symbols with their segment: text/data/bss."""
    d = open(path, 'rb').read()
    magic, t, dd, b, s = struct.unpack('>HIIII', d[:18])
    off, end = 0x1C + t + dd, 0x1C + t + dd + s
    out = []
    while off < end:
        name = d[off:off + 8].rstrip(b'\0').decode('ascii', 'replace')
        typ, val = struct.unpack('>HI', d[off + 8:off + 14])
        off += 14
        if not (typ & 0x8000):
            continue
        if typ & 0x0200:
            seg = 'text'
        elif typ & 0x0400:
            seg = 'data'
        elif typ & 0x0100:
            seg = 'bss'
        else:
            continue
        out.append((val, name, seg))
    return sorted(out), t, dd


def object_symbols():
    """Map each text symbol to its object file (link inputs)."""
    owner = {}
    for o in sorted(glob.glob(os.path.join(BUILD, '*.o'))):
        base = os.path.basename(o)
        if base in ('osbind.o', 'crt0.o', 'nofloat.o'):
            continue
        d = open(o, 'rb').read()
        if len(d) < 18 or d[:2] != b'\x60\x1a':
            continue
        magic, t, dd, b, s = struct.unpack('>HIIII', d[:18])
        off, end = 0x1C + t + dd, 0x1C + t + dd + s
        while off < end:
            name = d[off:off + 8].rstrip(b'\0').decode('ascii', 'replace')
            typ, val = struct.unpack('>HI', d[off + 8:off + 14])
            off += 14
            if (typ & 0x8000) and (typ & 0x0200):
                owner[name] = base
    return owner


def main():
    ptext, ptsize, prelocs = read_prg(PORT)
    otext, otsize, orelocs = read_prg(ORIG)
    allsyms, t, dd = read_all_syms(SYM68K)
    tsyms = [(v, n) for v, n, seg in allsyms if seg == 'text' and v < t]

    # symbol lookup for data/bss operands
    dsyms = sorted((v, n) for v, n, seg in allsyms if seg in ('data', 'bss'))

    def sym_at(addr):
        lo = None
        for v, n in dsyms:
            if v <= addr:
                lo = (n, addr - v, v)
            else:
                break
        return lo

    bounds = sorted(tsyms) + [(ptsize, '<end>')]
    text_map = {}                 # port name -> (rom_addr, size)
    votes = {}                    # (sym, off) -> {rom_addr: count}
    for k, (off, name) in enumerate(sorted(tsyms)):
        size = bounds[k + 1][0] - off
        if size <= 0:
            continue
        code = ptext[off:off + size]
        pat, fx = pattern(code, off, prelocs)
        if fx < MIN_UNIQUE:
            continue
        hits = [m.start() for m in re.finditer(pat, otext, re.DOTALL)]
        if len(hits) != 1:
            continue              # skip ambiguous twins for mapping
        oo = hits[0]
        text_map[name] = (oo, size)
        for o2, w, fix in tokens(code, off, prelocs):
            if fix or w != 4:
                continue
            pv = struct.unpack('>I', code[o2:o2 + 4])[0]
            ov = struct.unpack('>I', otext[oo + o2:oo + o2 + 4])[0]
            if pv < t:            # text-target operand (jsr): maps text
                continue
            hit = sym_at(pv)
            if hit is None:
                continue
            n, delta, base = hit
            key = (n, delta)
            votes.setdefault(key, {}).setdefault(ov - delta, 0)
            votes[key][ov - delta] += 1

    # collapse votes
    data_map, conflicts = {}, []
    for (n, delta), cand in votes.items():
        best = max(cand.items(), key=lambda kv: kv[1])
        data_map.setdefault(n, {}).setdefault(best[0], 0)
        data_map[n][best[0]] += best[1]
    final = {}
    for n, cand in data_map.items():
        addrs = sorted(cand.items(), key=lambda kv: -kv[1])
        final[n] = addrs[0][0]
        if len(addrs) > 1:
            conflicts.append((n, addrs))

    if '--data' in sys.argv:
        for n, a in sorted(final.items(), key=lambda kv: kv[1]):
            seg = 'data' if a < 0x141a2 else 'bss'
            print(f'0x{a:05x} {seg:4} {n}')
        return

    if '--objects' in sys.argv:
        owner = object_symbols()
        objs = {}
        for n, (oo, size) in text_map.items():
            o = owner.get(n, '?')
            objs.setdefault(o, []).append((oo, n, size))
        rows = []
        for o, fns in objs.items():
            fns.sort()
            rows.append((fns[0][0], o, fns[0][0], fns[-1][0] + fns[-1][2],
                         len(fns)))
        rows.sort()
        print('ROM link order (by first matched function):')
        for _, o, lo, hi, nf in rows:
            print(f'  0x{lo:05x}-0x{hi:05x}  {o:<14} {nf} fns')
        return

    print(f'{len(text_map)} uniquely-placed functions, '
          f'{len(final)} globals mapped, {len(conflicts)} conflicts')
    for n, addrs in conflicts[:20]:
        print('  CONFLICT', n, addrs)


if __name__ == '__main__':
    main()
