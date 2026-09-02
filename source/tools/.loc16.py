#!/usr/bin/env python3
"""stx_locate.py -- locate divergent functions in LCP_STX by matching
an INTERIOR window instead of the prologue.

verify_bytes hunts for a function with its first 24 bytes, and
stx_addrs.py reads callee addresses out of relocated call sites; both
fail for a function whose opening already differs and that nothing
matched calls.  But a long function usually still has one stretch that
survived the revision -- a run of stores to distinctive globals, a
string push, a jump table.  Sliding a window over the port's bytes and
searching LCP_STX for each (with the same relocation/branch wildcards
verify_bytes uses) finds those stretches; a window that occurs exactly
once pins the function.

Only unique hits are reported, and only when the implied function
start is not already claimed by a matched function.

CAVEAT: the reported start is `hit - window_offset`, which assumes the
bytes before the window are the same LENGTH in both revisions.  When
the prologue differs in size the start is off by that much, so an
address harvested from a relocated call site (stx_addrs.py) always
wins over one reported here.  Treat these as leads to hand to fn_diff,
and expect to nudge the address a few bytes.

Usage:
  python3 source/tools/stx_locate.py            # unlocated divergent
  python3 source/tools/stx_locate.py NAME ...   # just these
"""
import importlib.util
import io
import contextlib
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
REF = os.environ.get('LCP_REF', os.path.join(ROOT, 'DATA', 'LCP_STX.PRG'))

WINDOW = 16        # bytes per probe window
MIN_FIXED = 8     # need this many non-wildcarded bytes to be credible
STEP = 2


def load_vb():
    spec = importlib.util.spec_from_file_location(
        'vb', os.path.join(HERE, 'verify_bytes.py'))
    vb = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(vb)
    vb.ORIG = REF
    vb.KEPT_PREFIXES = tuple()
    vb.KEPT_NAMES = set()
    return vb


def sweep(vb):
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
    div, claimed, located = {}, [], set()
    cur = None
    for l in buf.getvalue().splitlines():
        m = re.match(r'MATCH\s+\S+\s+port=0x[0-9a-f]+\s+orig=0x([0-9a-f]+)'
                     r'\s+len=(\d+)', l)
        if m:
            claimed.append((int(m.group(1), 16), int(m.group(2))))
            continue
        m = re.match(r'DIVERGENT\s+_?(\S+)\s+port=0x([0-9a-f]+)\s+len=(\d+)', l)
        if m:
            cur = m.group(1)
            div[cur] = (int(m.group(2), 16), int(m.group(3)))
            continue
        if cur and 'candidate orig=' in l:
            located.add(cur)
            cur = None
    return div, claimed, located


def main():
    want = {a for a in sys.argv[1:] if not a.startswith('-')}
    vb = load_vb()
    ptext, _pt, prel = vb.read_prg(vb.PORT)
    otext, _ot, _orel = vb.read_prg(REF)
    div, claimed, located = sweep(vb)

    def is_claimed(a):
        return any(s <= a < s + n for s, n in claimed)

    hits = []
    for name, (poff, size) in sorted(div.items()):
        if want and name not in want:
            continue
        if not want and name in located:
            continue
        best = None
        for w in range(0, max(1, size - WINDOW), STEP):
            code = ptext[poff + w:poff + w + WINDOW]
            if len(code) < WINDOW:
                break
            pat, fixed = vb.pattern(code, poff + w, prel)
            if fixed < MIN_FIXED:
                continue
            found = [m.start() for m in re.finditer(pat, otext, re.DOTALL)]
            if len(found) == 1:
                start = found[0] - w
                if 0 <= start and not is_claimed(start):
                    best = (start, w)
                    break
        if best:
            hits.append((name, best[0], best[1]))

    print(f'{len(hits)} function(s) located by interior window '
          f'({WINDOW} bytes, >= {MIN_FIXED} fixed):')
    for name, start, w in sorted(hits, key=lambda h: h[1]):
        print(f'  {name:<12} {start:#07x}   (window at +0x{w:x})')


if __name__ == '__main__':
    main()
