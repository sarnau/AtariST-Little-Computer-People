#!/usr/bin/env python3
"""stx_objmap.py -- recover LCP_STX.PRG's object-file partition.

as68 shortens a call to a symbol defined in the SAME assembly unit
into `bsr`; a call to any other unit stays `jsr abs.l`.  The linker
lays each object down as ONE CONTIGUOUS text range.  Together those
two facts turn every bsr into a strong structural constraint:

    a bsr from A to B  =>  everything in [min(A,B), max(A,B)]
                           belongs to a single object

So merging the intervals spanned by all bsr instructions yields
lower bounds on the objects' extents; the gaps between merged
intervals are the only places an object boundary can fall.  That
recovers the STX build's source-file grouping wholesale, instead of
discovering it one regrouped function at a time.

Cross-checks reported:
  * jsr edges landing inside a merged cluster (would contradict the
    bsr evidence -- expected only for >32 KB intra-object calls,
    which as68 cannot shorten),
  * which port functions (byte-matched by verify_bytes) fall in each
    cluster, so clusters can be named after the port's .c files.

Usage:
  python3 source/tools/stx_objmap.py            # cluster report
  python3 source/tools/stx_objmap.py --edges    # + the bsr edges
"""
import io, os, re, struct, subprocess, sys, tempfile, contextlib
import importlib.util

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
REF = os.environ.get('LCP_REF', os.path.join(ROOT, 'DATA', 'LCP_STX.PRG'))
OBJDUMP = os.environ.get('OBJDUMP', 'm68k-elf-objdump')


def text_of(path):
    d = open(path, 'rb').read()
    magic, t, dd, b, s = struct.unpack('>HIIII', d[:18])
    assert magic == 0x601A, path
    return d[0x1C:0x1C + t]


def disassemble(text):
    with tempfile.NamedTemporaryFile(suffix='.bin', delete=False) as f:
        f.write(text)
        tmp = f.name
    try:
        out = subprocess.run(
            [OBJDUMP, '-D', '-b', 'binary', '-m', 'm68k', tmp],
            capture_output=True, text=True, check=True).stdout
    finally:
        os.unlink(tmp)
    return out


LINE = re.compile(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{4} )+)\s*(\S+)\s*(.*)$')
TARGET = re.compile(r'0x([0-9a-f]+)')


def scan(dis):
    """-> (function starts, bsr edges, jsr edges)"""
    starts, bsr, jsr = set(), [], []
    for line in dis.splitlines():
        m = LINE.match(line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        mnem, args = m.group(3), m.group(4)
        if mnem.startswith('link') and '%fp' in args:
            starts.add(addr)
        elif mnem.startswith('bsr'):
            t = TARGET.search(args)
            if t:
                tgt = int(t.group(1), 16)
                bsr.append((addr, tgt))
                starts.add(tgt)
        elif mnem.startswith('jsr'):
            t = TARGET.search(args)
            if t:
                jsr.append((addr, int(t.group(1), 16)))
    return sorted(starts), bsr, jsr


def merge(intervals):
    out = []
    for lo, hi in sorted(intervals):
        if out and lo <= out[-1][1]:
            out[-1][1] = max(out[-1][1], hi)
        else:
            out.append([lo, hi])
    return out


def matched_names():
    """{STX address: port name} for every byte-matched function."""
    spec = importlib.util.spec_from_file_location(
        'vb', os.path.join(os.path.dirname(os.path.abspath(__file__)),
                           'verify_bytes.py'))
    vb = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(vb)
    vb.ORIG = REF
    vb.KEPT_PREFIXES = tuple()
    vb.KEPT_NAMES = set()
    argv, sys.argv = sys.argv, ['verify_bytes.py', '-v']
    buf = io.StringIO()
    try:
        with contextlib.redirect_stdout(buf):
            try:
                vb.main()
            except SystemExit:
                pass
    finally:
        sys.argv = argv
    names = {}
    for l in buf.getvalue().splitlines():
        m = re.match(r'MATCH\s+(\S+)\s+port=0x[0-9a-f]+\s+orig=0x([0-9a-f]+)', l)
        if m:
            names[int(m.group(2), 16)] = m.group(1)
    return names


def port_objects():
    """{port function name: owning .o} from the build tree symbol tables."""
    build = os.path.join(ROOT, 'source', 'build', 'alcyon')
    out = {}
    for f in sorted(os.listdir(build)):
        if not f.endswith('.o'):
            continue
        d = open(os.path.join(build, f), 'rb').read()
        if len(d) < 18 or d[:2] != b'\x60\x1a':
            continue
        _, t, dd, b, s = struct.unpack('>HIIII', d[:18])
        off = 0x1C + t + dd
        i = off
        while i + 14 <= off + s:
            name = d[i:i + 8].rstrip(b'\0').decode('latin1')
            typ, val = struct.unpack('>HI', d[i + 8:i + 14])
            i += 14
            if (typ & 0xA200) == 0xA200 and not (typ & 0x0800):
                out[name] = f
    return out


def main():
    show_edges = '--edges' in sys.argv
    text = text_of(REF)
    starts, bsr, jsr = scan(disassemble(text))
    print(f'{REF}: {len(text)} text bytes, {len(starts)} function starts, '
          f'{len(bsr)} bsr, {len(jsr)} jsr')

    clusters = merge([[min(a, b), max(a, b)] for a, b in bsr])
    names = matched_names()
    owners = port_objects()

    print(f'\n{len(clusters)} bsr clusters (lower bounds on object extents):')
    for lo, hi in clusters:
        inside = [n for a, n in sorted(names.items()) if lo <= a <= hi]
        span = hi - lo
        objs = []
        for n in inside:
            o = owners.get(n)
            if o and o not in objs:
                objs.append(o)
        label = ', '.join(inside[:5]) + (' ...' if len(inside) > 5 else '')
        print(f'  {lo:#07x}-{hi:#07x}  {span:6d} B  {len(inside)} known: {label}')
        if objs:
            print(f'{"":>22}port objects: {" ".join(objs)}')

    bad = [(a, b) for a, b in jsr
           if any(lo <= min(a, b) and max(a, b) <= hi for lo, hi in clusters)]
    print(f'\njsr edges inside a cluster (expect 0 unless >32 KB): {len(bad)}')
    for a, b in bad[:10]:
        print(f'  {a:#07x} -> {b:#07x}  (distance {abs(b - a):#x})')

    if show_edges:
        print('\nbsr edges:')
        for a, b in sorted(bsr):
            print(f'  {a:#07x} -> {b:#07x}')


if __name__ == '__main__':
    main()
