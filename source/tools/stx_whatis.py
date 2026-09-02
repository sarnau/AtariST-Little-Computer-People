#!/usr/bin/env python3
"""stx_whatis.py ADDR [ADDR...] -- which port function is this?

Given an address in LCP_STX, tries every divergent port function
there and reports the ones whose prefix survives longest.  Useful once
a neighbouring match pins where a function must start but the order
inside the object is unknown.
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
    spec = importlib.util.spec_from_file_location(
        'vb', os.path.join(HERE, 'verify_bytes.py'))
    vb = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(vb)
    vb.ORIG = REF
    vb.KEPT_PREFIXES = tuple()
    vb.KEPT_NAMES = set()
    ptext, _pt, prel = vb.read_prg(vb.PORT)
    otext, _ot, _orel = vb.read_prg(REF)

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
    div = [(m.group(1), int(m.group(2), 16), int(m.group(3)))
           for m in (re.match(
               r'DIVERGENT\s+_?(\S+)\s+port=0x([0-9a-f]+) len=(\d+)', l)
               for l in buf.getvalue().splitlines()) if m]

    def stx_len(a):
        p = a + 4
        while p + 2 <= len(otext):
            if otext[p:p + 2] == b'\x4e\x56' and \
                    otext[p - 2:p] in (b'\x4e\x75', b'\x4e\x71'):
                return p - a
            p += 2
        return 0

    for arg in argv[1:]:
        if arg.startswith('-'):
            continue
        a = int(arg, 0)
        olen = stx_len(a)
        best = []
        for name, poff, size in div:
            code = ptext[poff:poff + size]
            _j_any, j = vb.first_mismatch(code, poff, prel, otext, a)
            good = size if j < 0 else j
            # token similarity over the shorter of the two spans
            n = min(size, olen) if olen else size
            same = sum(1 for off, w, fix in vb.tokens(code[:n], poff, prel)
                       if fix and code[off:off + w] ==
                       otext[a + off:a + off + w])
            score = same / max(1, n)
            best.append((abs(size - olen) if olen else 9999,
                         -score, good, name, size))
        best.sort()
        print(f'{a:#07x}: STX length {olen}')
        for dl, nscore, good, name, size in best[:6]:
            print(f'    {name:<10} len {size:<5} dlen {dl:<5}'
                  f' similarity {-nscore:.2f}  prefix {good}')


if __name__ == '__main__':
    main()
