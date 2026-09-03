#!/usr/bin/env python3
"""stx_txtdiff.py -- whole-TEXT comparison against the reference binary.

verify_bytes.py wildcards relocations AND PC-relative displacements, so
a function can be reported MATCH while its internal branch targets --
and therefore the object's real bytes -- still differ.  This tool
wildcards relocations ONLY, and walks the whole text segment, so it is
what the byte-identity phase iterates on.

    python3 source/tools/stx_txtdiff.py            # first difference
    python3 source/tools/stx_txtdiff.py 0x8000     # start further in
    python3 source/tools/stx_txtdiff.py 0x81ae 1330  # one function

Honours LCP_REF like the other tools; defaults to DATA/LCP_STX.PRG.
"""
import sys, os

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__))))
import verify_bytes as vb

vb.ORIG = os.environ.get('LCP_REF', 'DATA/LCP_STX.PRG')
p, pts, prel = vb.read_prg(vb.PORT)
o, ots, orel = vb.read_prg(vb.ORIG)

skip = set()
for r in set(prel) | set(orel):
    for k in range(4):
        skip.add(r + k)

start = int(sys.argv[1], 0) if len(sys.argv) > 1 else 0
count = int(sys.argv[2], 0) if len(sys.argv) > 2 else None
end = min(len(p), len(o)) if count is None else start + count

diffs = [i for i in range(start, end) if i not in skip and p[i] != o[i]]
print("port text %d, orig text %d, delta %+d" % (pts, ots, pts - ots))
print("%d differing bytes in 0x%x..0x%x" % (len(diffs), start, end))
for i in diffs[:12]:
    a = i & ~7
    print("  0x%05x" % i)
    print("     port %s" % p[a:a + 24].hex(' '))
    print("     orig %s" % o[a:a + 24].hex(' '))
