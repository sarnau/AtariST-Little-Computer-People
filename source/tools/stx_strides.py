#!/usr/bin/env python3
"""stx_strides.py -- recover array row strides from the disassembly.

Alcyon indexes `T a[N][M]` as `base + i*sizeof(T[M])`, and it emits that
scale as a literal `muls.w #K,Dn` (or, for a power of two, a chain of
`add.l An,An` doublings / an `lsl.l`).  So the ROW STRIDE of every
2-dimensional array in the program is written down in the text segment
and can be read back without knowing anything about the source.

That is worth having twice over:

  * it CHECKS a declaration -- if the port says `char a[N][M]` and the
    text multiplies by something other than M, one of them is wrong;
  * it SIZES an array when combined with the loop bound that feeds the
    index, which is the other half of the layout evidence (the first
    half being the relocation gaps -- see stx_map.py / CLAUDE.md).

Usage:
    python3 source/tools/stx_strides.py          # every array
    python3 source/tools/stx_strides.py _pex_ptr # one symbol, w/ sites
"""
import bisect, collections, os, struct, sys

ROOT = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, ROOT)
import verify_bytes as vb

PRG = os.path.join(ROOT, '..', 'build', 'alcyon', 'LCP.PRG')
SYM = os.path.join(ROOT, '..', 'build', 'alcyon', 'lcp_sym.68k')

WINDOW = 52     # how far after the scale a base reference may sit


def load(path):
    d = open(path, 'rb').read()
    _, t, dd, b, ss = struct.unpack('>HIIII', d[:18])
    return t, dd, d[28:28 + t], d[28 + t + dd + ss:]


def relocs(t, dd, relbytes):
    out = []
    if len(relbytes) >= 4:
        off = struct.unpack('>I', relbytes[:4])[0]
        if off:
            out.append(off)
            i = 4
            while i < len(relbytes):
                c = relbytes[i]; i += 1
                if c == 0:
                    break
                off += 254 if c == 1 else c
                if c != 1:
                    out.append(off)
    return [o for o in out if o < t]


def main():
    t, dd, text, relbytes = load(PRG)
    sites = relocs(t, dd, relbytes)
    syms = {}
    d = open(SYM, 'rb').read()
    _, st, sd, sb, ss = struct.unpack('>HIIII', d[:18])
    tabl = d[28 + st + sd:28 + st + sd + ss]
    for i in range(0, len(tabl), 14):
        name = tabl[i:i + 8].rstrip(b'\0').decode('latin1')
        val = struct.unpack('>I', tabl[i + 8 + 2:i + 14])[0]
        syms.setdefault(val, name)
    addrs = sorted(syms)

    def owner(v):
        i = bisect.bisect_right(addrs, v) - 1
        return (syms[addrs[i]], v - addrs[i]) if i >= 0 else (None, None)

    want = sys.argv[1] if len(sys.argv) > 1 else None
    found = collections.defaultdict(collections.Counter)
    where = collections.defaultdict(list)
    for i in range(0, t - 4, 2):
        w = struct.unpack('>H', text[i:i + 2])[0]
        if (w & 0xF1FF) == 0xC1FC:          # muls.w #imm,Dn
            k = struct.unpack('>H', text[i + 2:i + 4])[0]
            skip = 4
        elif (w & 0xF1F8) in (0xE180, 0xE188):   # asl.l / lsl.l #n,Dn
            n = (w >> 9) & 7
            k = 1 << (8 if n == 0 else n)
            skip = 2
        else:
            continue
        j = bisect.bisect_left(sites, i + skip)
        if j >= len(sites) or sites[j] >= i + WINDOW:
            continue
        val = struct.unpack('>I', text[sites[j]:sites[j] + 4])[0]
        n, off = owner(val)
        if n:
            found[n][k] += 1
            where[(n, k)].append(i)

    for n in sorted(found):
        if want and n != want:
            continue
        c = found[n]
        best, cnt = c.most_common(1)[0]
        flag = '' if len(c) == 1 else '   (also %s)' % {k: v for k, v in c.items() if k != best}
        print("%-12s stride %-6d %3d site(s)%s" % (n, best, cnt, flag))
        if want:
            for a in where[(n, best)]:
                print("      muls.w #%d at text 0x%05x" % (best, a))


main()
