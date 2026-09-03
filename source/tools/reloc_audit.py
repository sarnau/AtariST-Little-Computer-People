#!/usr/bin/env python3
"""reloc_audit.py -- audit every relocation TARGET against the reference.

verify_bytes.py and stx_txtdiff.py both wildcard relocated longwords,
and prg_diff.py compares the binary AFTER bss_remap has rewritten them,
so neither can tell you that a function loads the wrong ADDRESS.  This
tool pairs the two relocation streams site by site -- they are
identical, because the text and data are -- and reports every way the
port's variable structure disagrees with the original's:

  A  one port cell -> several reference cells
     The port MERGED two of the original's variables (mi_nlp0 hid
     mi_ndur this way).
  B  several port cells -> one reference cell
     The original ALIASED storage the port keeps apart.  One is
     expected: last_hz and the sequencer's mi_lasT really are one cell.
  C  one port symbol spanning two reference variables.
  D  port BSS symbols nothing relocates against -- the pairing cannot
     see them, so their size and existence are unverifiable here.
  E  symbols whose base is INFERRED from a non-zero offset.  Their
     declared shape is a guess: `aes_intO[7]` and a plain short emit
     the same instruction, because Alcyon folds a constant subscript
     into the absolute address.
  F  port symbols declared LARGER than the gap to the next cell the
     reference actually uses -- an over-declared array (or, for
     last_hz, the alias from B).

Segment and text/data-target mismatches are fatal and printed first.

Reads the PRE-REMAP binary (build/alcyon/LCP_nobss.PRG, which
alcyon_link.sh leaves behind) and its side link lcp_sym.68k.  Honours
LCP_REF; the default reference is DATA/LCP_STX.PRG.

    python3 source/tools/reloc_audit.py [LCP_nobss.PRG] [reference.prg]
"""
import os, struct, sys, bisect, collections

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PORT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        ROOT, 'build', 'alcyon', 'LCP_nobss.PRG')
REF  = sys.argv[2] if len(sys.argv) > 2 else os.environ.get(
        'LCP_REF', os.path.join(ROOT, '..', 'DATA', 'LCP_STX.PRG'))
SYM  = os.path.join(os.path.dirname(PORT) or '.', 'lcp_sym.68k')


def load(path):
    d = open(path, 'rb').read()
    magic, t, dd, b, s = struct.unpack('>HIIII', d[:18])
    assert magic == 0x601A, path
    rel = d[0x1C + t + dd + s:]
    sites = []
    if len(rel) >= 4:
        pos = struct.unpack('>I', rel[:4])[0]
        if pos:
            sites.append(pos)
            i = 4
            while i < len(rel):
                c = rel[i]; i += 1
                if c == 0:
                    break
                pos += 254 if c == 1 else c
                if c != 1:
                    sites.append(pos)
    return t, dd, b, d[0x1C:0x1C + t + dd], sites


def syms(path, kind):
    d = open(path, 'rb').read()
    _, t, dd, b, s = struct.unpack('>HIIII', d[:18])
    off = 0x1C + t + dd
    out = []
    i = off
    while i < off + s:
        name = d[i:i + 8].rstrip(b'\0 ').decode('latin1')
        typ, val = struct.unpack('>HI', d[i + 8:i + 14])
        i += 14
        if typ == kind:
            out.append((val, name))
    out.sort()
    return out


pt, pd, pbs, pb, ps = load(PORT)
ot, od, obs, ob, os_ = load(REF)
if ps != os_:
    sys.exit('relocation streams differ -- fix text/data first')

tx = syms(SYM, 0xA200)
bs = syms(SYM, 0xA100)
ta = [v for v, _ in tx]
ba = [v for v, _ in bs]
pbase, obase = pt + pd, ot + od


def where(site):
    if site >= pt:
        return 'data+0x%x' % (site - pt)
    k = bisect.bisect_right(ta, site) - 1
    return '%s+0x%x' % (tx[k][1], site - ta[k])


bad = []
cells = collections.defaultdict(set)
refs = collections.defaultdict(list)
for site in ps:
    pv = struct.unpack('>I', pb[site:site + 4])[0]
    ov = struct.unpack('>I', ob[site:site + 4])[0]
    pseg = 0 if pv < pt else (1 if pv < pbase else 2)
    oseg = 0 if ov < ot else (1 if ov < obase else 2)
    if pseg != oseg or (pseg < 2 and pv != ov):
        bad.append((site, pv, ov))
        continue
    if pseg == 2:
        k = bisect.bisect_right(ba, pv) - 1
        key = (bs[k][1], pv - ba[k])
        cells[key].add(ov - obase)
        refs[key].append(site)

fail = 0
print('segment / text / data target mismatches: %d' % len(bad))
for site, pv, ov in bad:
    print('   %-22s port %#x   reference %#x' % (where(site), pv, ov))
fail += len(bad)

bysym = collections.defaultdict(dict)
for (name, off), v in cells.items():
    bysym[name][off] = sorted(v)[0]

A = [(k, v) for k, v in sorted(cells.items()) if len(v) > 1]
print('\nA. one port cell -> several reference cells: %d' % len(A))
for k, v in A:
    print('   %s+0x%x -> %s' % (k[0], k[1], [hex(x) for x in sorted(v)]))
    for site in sorted(set(refs[k])):
        print('        via %s -> %#x' % (
            where(site),
            struct.unpack('>I', ob[site:site + 4])[0] - obase))
fail += len(A)

rev = collections.defaultdict(set)
for k, v in cells.items():
    for a in v:
        rev[a].add(k)
B = [(a, ks) for a, ks in sorted(rev.items()) if len(ks) > 1]
print('\nB. several port cells -> one reference cell (alias): %d' % len(B))
for a, ks in B:
    print('   reference +%#08x <- %s' % (
        a, ', '.join('%s+0x%x' % k for k in sorted(ks))))
    for k in sorted(ks):
        print('        %s+0x%x via %s' % (
            k[0], k[1], ', '.join(sorted({where(x) for x in refs[k]}))))

C = [(n, o) for n, o in sorted(bysym.items())
     if len({a - x for x, a in o.items()}) > 1]
print('\nC. one port symbol spanning two reference variables: %d' % len(C))
for n, o in C:
    print('   %s bases %s' % (n, sorted({hex(a - x) for x, a in o.items()})))
fail += len(C)

size = {}
for i, (v, n) in enumerate(bs):
    size[n] = (bs[i + 1][0] - v) if i + 1 < len(bs) else 0
D = [(v, n) for v, n in bs if n not in bysym]
print('\nD. port BSS symbols with no relocation (unverifiable): %d of %d'
      % (len(D), len(bs)))
for v, n in D:
    print('   %-10s port %#08x size %d' % (n, v - pbase, size[n]))

E = [(n, min(o)) for n, o in sorted(bysym.items()) if 0 not in o]
print('\nE. symbols whose base is inferred from a non-zero offset: %d' % len(E))
for n, o in E:
    print('   %-10s first referenced at +%d' % (n, o))

base = {n: sorted({a - x for x, a in o.items()})[0] for n, o in bysym.items()}
rows = sorted((b, n) for n, b in base.items())
F = []
for i, (b, n) in enumerate(rows):
    if i + 1 < len(rows) and size[n] > rows[i + 1][0] - b:
        F.append((n, b, size[n], rows[i + 1][1], rows[i + 1][0] - b))
print('\nF. declared size > gap to the next cell the reference uses: %d' % len(F))
for n, b, sz, nxt, gap in F:
    print('   %-10s remapped %#08x  declared %-6d  next %-10s at +%-5d'
          '  over by %d' % (n, b, sz, nxt, gap, sz - gap))

sys.exit(1 if fail else 0)
