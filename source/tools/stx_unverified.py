#!/usr/bin/env python3
"""stx_unverified.py -- what verify_bytes never looks at.

verify_bytes walks the port's SYMBOL table, so a `static` function --
which Alcyon emits without a symbol -- is never compared against the
reference at all.  This tool marks every byte covered by a matched
function and prints the runs that are left inside the game-code range,
i.e. the static helpers whose source shape is still unproven.

    python3 source/tools/stx_unverified.py

Honours LCP_REF; defaults to DATA/LCP_STX.PRG.
"""
import sys, os, io, contextlib, importlib.util

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
spec = importlib.util.spec_from_file_location('vb', os.path.join(HERE, 'verify_bytes.py'))
vb = importlib.util.module_from_spec(spec)
spec.loader.exec_module(vb)
vb.ORIG = os.environ.get('LCP_REF', 'DATA/LCP_STX.PRG')
vb.KEPT_PREFIXES = tuple()
vb.KEPT_NAMES = set()

LO, HI = 0x12a, 0x1733a          # the game's own code; the rest is library

sys.argv = ['verify_bytes.py', '-v']
buf = io.StringIO()
with contextlib.redirect_stdout(buf):
    try:
        vb.main()
    except SystemExit:
        pass

cov = bytearray(HI)
for line in buf.getvalue().splitlines():
    if not line.startswith('MATCH'):
        continue
    f = line.split()
    a = int(f[3].split('=')[1], 16)
    n = int(f[4].split('=')[1])
    for i in range(a, min(a + n, HI)):
        cov[i] = 1

runs = []
i = LO
while i < HI:
    if cov[i]:
        i += 1
        continue
    j = i
    while j < HI and not cov[j]:
        j += 1
    runs.append((i, j - i))
    i = j

total = sum(n for _, n in runs)
print("game code 0x%05x..0x%05x = %d bytes" % (LO, HI, HI - LO))
print("UNVERIFIED %d bytes (%.1f%%) in %d runs:" % (total, 100.0 * total / (HI - LO), len(runs)))
for a, n in sorted(runs, key=lambda r: -r[1]):
    print("   0x%05x  %5d bytes" % (a, n))
