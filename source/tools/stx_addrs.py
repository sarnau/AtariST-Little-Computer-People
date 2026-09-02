#!/usr/bin/env python3
"""stx_addrs.py -- recover STX addresses for functions verify_bytes
cannot locate on its own.

verify_bytes finds a function by searching LCP_STX for its byte
pattern, so a function whose FIRST instructions already diverge has
"no candidate" and cannot be diffed at all -- which is the state of
most of the remaining work.

But a matched function pins its callees for free: at every relocated
call site inside it, the port operand names the callee and the STX
operand at the SAME site is that callee's STX address.  Aggregating
over all matched functions yields a callee -> STX address table, and
those addresses can be handed straight to

    LCP_REF=DATA/LCP_STX.PRG fn_diff.py <name> <addr>

Conflicting evidence (one port symbol mapping to two STX addresses)
is reported rather than guessed at -- it would mean a mis-identified
match.

Usage:
  python3 source/tools/stx_addrs.py           # unlocated functions only
  python3 source/tools/stx_addrs.py --all     # every callee found
"""
import bisect, io, contextlib, importlib.util, os, re, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
REF = os.environ.get('LCP_REF', os.path.join(ROOT, 'DATA', 'LCP_STX.PRG'))
BUILD = os.path.join(ROOT, 'source', 'build', 'alcyon')


def load(path):
    d = open(path, 'rb').read()
    _m, t, dd, b, s = struct.unpack('>HIIII', d[:18])
    return d[0x1C:0x1C + t + dd], d[0x1C + t + dd + s:], t


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


def port_syms():
    d = open(os.path.join(BUILD, 'lcp_sym.68k'), 'rb').read()
    _m, t, dd, _b, s = struct.unpack('>HIIII', d[:18])
    off = 0x1C + t + dd
    out = []
    i = off
    while i + 14 <= off + s:
        name = d[i:i + 8].rstrip(b'\0').decode('latin1')
        typ, val = struct.unpack('>HI', d[i + 8:i + 14])
        i += 14
        if typ == 0xA200:                      # defined text symbol
            out.append((val, name))
    out.sort()
    return out


def sweep():
    """-> (matched {name: (port_off, size, stx_off)}, divergent names)"""
    spec = importlib.util.spec_from_file_location(
        'vb', os.path.join(HERE, 'verify_bytes.py'))
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
    matched, divergent, located = {}, [], set()
    cur = None
    for l in buf.getvalue().splitlines():
        m = re.match(r'MATCH\s+(\S+)\s+port=0x([0-9a-f]+)\s+orig=0x([0-9a-f]+)'
                     r'\s+len=(\d+)', l)
        if m:
            matched[m.group(1)] = (int(m.group(2), 16), int(m.group(4)),
                                   int(m.group(3), 16))
            continue
        m = re.match(r'DIVERGENT\s+(\S+)', l)
        if m:
            cur = m.group(1); divergent.append(cur); continue
        if cur and 'candidate orig=' in l:
            located.add(cur); cur = None
    return matched, divergent, located


def main():
    show_all = '--all' in sys.argv
    pd, prel, pt = load(os.path.join(BUILD, 'LCP.PRG'))
    od, _orel, _ot = load(REF)
    syms = port_syms()
    addrs = [v for v, _ in syms]
    names = [n for _, n in syms]

    def sym_at(a):
        j = bisect.bisect_right(addrs, a) - 1
        return (names[j], a - addrs[j]) if j >= 0 else (None, None)

    matched, divergent, located = sweep()
    # port text offset -> (name, size, stx offset) for matched functions
    spans = sorted((v[0], v[1], v[2], k) for k, v in matched.items())
    starts = [s[0] for s in spans]

    found, conflict = {}, {}
    for pos in sites(prel):
        j = bisect.bisect_right(starts, pos) - 1
        if j < 0:
            continue
        p0, size, o0, _fn = spans[j]
        if not (p0 <= pos < p0 + size):
            continue                            # not inside a matched fn
        pv = struct.unpack('>I', pd[pos:pos + 4])[0]
        ov = struct.unpack('>I', od[pos - p0 + o0:pos - p0 + o0 + 4])[0]
        if pv >= pt:                            # data/bss target
            continue
        name, off = sym_at(pv)
        if not name or off != 0:
            continue
        if name in found and found[name] != ov:
            conflict.setdefault(name, {found[name]}).add(ov)
        found[name] = ov

    want = [n for n in divergent if n not in located]
    rows = [(found[n], n) for n in (found if show_all else want) if n in found]
    rows.sort()
    print(f'{len(rows)} STX addresses recovered from matched call sites'
          f'{"" if show_all else " (unlocated divergent functions only)"}:')
    for a, n in rows:
        print(f'  {n:<12} {a:#07x}')
    if conflict:
        print(f'\nCONFLICTS ({len(conflict)}) -- a match may be wrong:')
        for n, vs in conflict.items():
            print(f'  {n}: {" ".join(hex(v) for v in sorted(vs))}')


if __name__ == '__main__':
    main()
