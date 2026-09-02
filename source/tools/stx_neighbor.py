#!/usr/bin/env python3
"""stx_neighbor.py -- infer a divergent function's STX address from the
matched function that precedes it in the SAME object.

Where a unity unit already reproduces LCP_STX's function order, the
function right after a matched one starts where that match ends.  This
prints, for every divergent function, the address implied by its
nearest matched predecessor -- and flags whether LCP_STX really has a
`link a6` prologue there, which is the sanity check that the order
holds.

Usage: python3 source/tools/stx_neighbor.py [NAME ...]
"""
import contextlib
import importlib.util
import io
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
REF = os.environ.get('LCP_REF', os.path.join(ROOT, 'DATA', 'LCP_STX.PRG'))


def main():
    want = {a for a in sys.argv[1:] if not a.startswith('-')}
    spec = importlib.util.spec_from_file_location(
        'vb', os.path.join(HERE, 'verify_bytes.py'))
    vb = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(vb)
    vb.ORIG = REF
    vb.KEPT_PREFIXES = tuple()
    vb.KEPT_NAMES = set()
    otext, _ot, _orel = vb.read_prg(REF)
    ptext, _pt, prel = vb.read_prg(vb.PORT)

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

    rows = []
    for l in buf.getvalue().splitlines():
        m = re.match(r'MATCH\s+_?(\S+)\s+port=0x([0-9a-f]+)\s+'
                     r'orig=0x([0-9a-f]+)\s+len=(\d+)', l)
        if m:
            rows.append(('M', m.group(1), int(m.group(2), 16),
                         int(m.group(3), 16), int(m.group(4))))
            continue
        m = re.match(r'DIVERGENT\s+_?(\S+)\s+port=0x([0-9a-f]+)\s+len=(\d+)', l)
        if m:
            rows.append(('D', m.group(1), int(m.group(2), 16), None,
                         int(m.group(3))))
    rows.sort(key=lambda r: r[2])

    print('divergent functions with an STX address implied by the '
          'preceding match:')
    prev = None
    n = 0
    for kind, name, poff, ooff, size in rows:
        if kind == 'M':
            prev = (name, ooff + size, poff + size)
            continue
        if prev is None:
            continue
        pname, oend, pend = prev
        prev = None                     # only the immediate successor
        if want and name not in want:
            continue
        gap = poff - pend
        a = oend + gap
        ok = otext[a:a + 2] == b'\x4e\x56'
        code = ptext[poff:poff + size]
        _j_any, j = vb.first_mismatch(code, poff, prel, otext, a)
        good = size if j < 0 else j
        if not ok:
            continue                    # not a function start at all
        n += 1
        print(f'  {name:<10} 0x{a:05x}  after {pname}'
              f'   (prefix {good}/{size})')
    print(f'{n} credible candidate(s)')


if __name__ == '__main__':
    main()
