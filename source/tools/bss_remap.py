#!/usr/bin/env python3
"""bss_remap.py -- re-lay the build's BSS to LCP_STX.PRG's layout.

The 1985 linker allocated `.comm` blocks in an order none of the
surviving toolchain linkers reproduce (native lo68/link68 allocate
hash-grouped, the 1990 ALN.PRG allocates alphabetically; the
reference's order matches neither, nor first-mention order, and it
does not even align its commons -- scrbufA lands on an odd address).
TEXT, DATA and the relocation stream come out byte-identical anyway;
the only bytes that differ are relocated longwords whose targets sit
in BSS, because the two linkers packed the same commons at different
offsets.

The reference allocation is therefore carried as a checked-in layout
spec, tools/stx_bss_layout.tsv: one row per referenced BSS cell,
keyed by PORT SYMBOL + OFFSET (stable across relinks) with the address
it must land on.  The normal link-time invocation

    python3 tools/bss_remap.py LCP.PRG

reads the spec, resolves the symbols against the unstripped side
link (lcp_sym.68k), rewrites every relocated BSS longword, and sets
the header BSS size from the spec.  The reference binary is NOT read.

Regenerate the spec (only needed after a layout-affecting source
change) with the reference binary present:

    python3 tools/bss_remap.py --gen LCP.PRG [reference.prg]

--gen pairs the two byte-identical relocation streams site-by-site
and verifies the port->reference translation is a consistent
one-to-one mapping before writing the table; that consistency check
is the proof that the port's reference structure (which symbol each
site addresses, at which offset) matches the original's exactly.

Both modes exit non-zero on any inconsistency, including a reloc
site whose target is missing from the spec (= layout drift:
regenerate with --gen and review the diff).
"""
import os, struct, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TSV  = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                    'stx_bss_layout.tsv')

GEN = '--gen' in sys.argv
argv = [a for a in sys.argv if a != '--gen']
PORT = argv[1] if len(argv) > 1 else os.path.join(
        ROOT, 'build', 'alcyon', 'LCP.PRG')
ORIG = argv[2] if len(argv) > 2 else os.path.join(
        ROOT, '..', 'DATA', 'LCP_STX.PRG')
SYM  = os.path.join(os.path.dirname(PORT) or '.', 'lcp_sym.68k')


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


def load_prg(path):
    d = open(path, 'rb').read()
    magic, t, dd, b, s = struct.unpack('>HIIII', d[:18])
    assert magic == 0x601A, path
    return d, t, dd, b, s


def load_syms(path):
    """Global symbols of the unstripped side link, sorted by address."""
    d = open(path, 'rb').read()
    _, t, dd, b, s = struct.unpack('>HIIII', d[:18])
    off = 0x1C + t + dd
    out = []
    i = off
    while i < off + s:
        name = d[i:i + 8].rstrip(b'\0').decode('latin1')
        typ, val = struct.unpack('>HI', d[i + 8:i + 14])
        i += 14
        if typ in (0xA100, 0xA200, 0xA400):     # bss / text / data global
            out.append((val, name))
    out.sort()
    return out


def resolver(syms):
    import bisect
    addrs = [v for v, _ in syms]
    names = [n for _, n in syms]

    def sym_of(a):
        j = bisect.bisect_right(addrs, a) - 1
        if j < 0:
            sys.exit(f'no symbol at or below {a:#x} in lcp_sym.68k')
        return names[j], a - addrs[j]
    return sym_of


def port_bss_targets(pd, t, dd, s):
    """{port BSS address: [site,...]} over the port's reloc stream."""
    preloc = pd[0x1C + t + dd + s:]
    out = {}
    for pos in sites(preloc):
        pv = struct.unpack('>I', pd[0x1C + pos:0x1C + pos + 4])[0]
        if pv >= t + dd:
            out.setdefault(pv, []).append(pos)
    return out, preloc


def gen():
    pd, pt, pdd, pb, ps = load_prg(PORT)
    od, ot, odd, ob, osz = load_prg(ORIG)
    if (pt, pdd) != (ot, odd):
        sys.exit('text/data sizes differ -- fix those before --gen')
    preloc = pd[0x1C + pt + pdd + ps:]
    oreloc = od[0x1C + ot + odd + osz:]
    if preloc != oreloc:
        sys.exit('relocation streams differ -- --gen needs them identical')

    mapping = {}
    for pos in sites(preloc):
        pv = struct.unpack('>I', pd[0x1C + pos:0x1C + pos + 4])[0]
        ov = struct.unpack('>I', od[0x1C + pos:0x1C + pos + 4])[0]
        if pv < pt + pdd and ov < pt + pdd:
            if pv != ov:
                sys.exit(f'text/data-target mismatch at site +{pos:#x}: '
                         f'port {pv:#x} orig {ov:#x}')
            continue
        if (pv < pt + pdd) != (ov < pt + pdd):
            sys.exit(f'segment mismatch at site +{pos:#x}: '
                     f'port {pv:#x} orig {ov:#x}')
        if pv in mapping and mapping[pv] != ov:
            sys.exit(f'inconsistent mapping for port {pv:#x}: '
                     f'{mapping[pv]:#x} vs {ov:#x} at site +{pos:#x}')
        mapping[pv] = ov
    # Many-to-one IS allowed: the original aliased storage in places the
    # port cannot spell in C without changing the codegen (last_hz and
    # the sequencer's mi_lasT are one cell there).  One-to-many is not,
    # and the loop above already rejects it.
    rev = {}
    for k, v in sorted(mapping.items()):
        rev.setdefault(v, []).append(k)
    for v, ks in sorted(rev.items()):
        if len(ks) > 1:
            print('  alias: ' + ', '.join(f'{k:#x}' for k in ks) +
                  f' -> {v:#x}')

    sym_of = resolver(load_syms(SYM))
    rows = []
    for pv, ov in mapping.items():
        name, off = sym_of(pv)
        rows.append((ov, name, off))
    rows.sort()
    with open(TSV, 'w') as f:
        f.write('# stx_bss_layout.tsv -- the original linker\'s BSS '
                'allocation, keyed by port symbol+offset.\n'
                '# Generated by bss_remap.py --gen against '
                'DATA/LCP_STX.PRG; regenerate after any\n'
                '# layout-affecting source change and review the '
                'diff.  Columns: symbol offset address.\n')
        f.write(f'# bss_size {ob}\n')
        for ov, name, off in rows:
            f.write(f'{name}\t{off:#x}\t{ov:#x}\n')
    print(f'bss_remap --gen: wrote {len(rows)} rows -> {TSV}')


def apply():
    pd, pt, pdd, pb, ps = load_prg(PORT)
    pd = bytearray(pd)

    rom_size = None
    spec = {}                       # (symbol, offset) -> rom address
    for line in open(TSV):
        if line.startswith('# bss_size'):
            rom_size = int(line.split()[2])
        if line.startswith('#') or not line.strip():
            continue
        name, off, ov = line.split()
        spec[(name, int(off, 16))] = int(ov, 16)
    if rom_size is None:
        sys.exit(f'{TSV}: missing "# bss_size" header')

    sym_of = resolver(load_syms(SYM))
    targets, preloc = port_bss_targets(pd, pt, pdd, ps)

    mapping = {}
    for pv in targets:
        key = sym_of(pv)
        if key not in spec:
            sys.exit(f'port BSS target {pv:#x} = {key[0]}+{key[1]:#x} '
                     f'not in {os.path.basename(TSV)} -- layout drift; '
                     f'regenerate with --gen and review the diff')
        mapping[pv] = spec[key]
    n = 0
    for pos in sites(preloc):
        pv = struct.unpack('>I', pd[0x1C + pos:0x1C + pos + 4])[0]
        if pv in mapping:
            pd[0x1C + pos:0x1C + pos + 4] = struct.pack('>I', mapping[pv])
            n += 1
    pd[10:14] = struct.pack('>I', rom_size)
    open(PORT, 'wb').write(pd)
    print(f'bss_remap: {len(mapping)} BSS addresses, {n} sites rewritten, '
          f'bss size {pb} -> {rom_size}')


if __name__ == '__main__':
    gen() if GEN else apply()
