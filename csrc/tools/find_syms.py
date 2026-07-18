#!/usr/bin/env python3
"""find_syms.py -- compute BASE-relative RAM addresses for DRI globals.

Walks the .o files in the same order alcyon_link.sh links them, then
returns each named symbol's BASE-relative offset in the final PRG.

Usage:
  python3 tools/find_syms.py [SYM1 SYM2 ...]

With no args, prints every global symbol.  DRI-style names begin with
an underscore -- pass `_lcp_x`, not `lcp_x`.

The output BASE-relative offset is what the Hatari-driven tests
(test_stairs.sh, test_stairs_up.sh) add to their `BASE` variable to
form the runtime RAM address.

Re-run this whenever globals.c / sprglobs.c / any .o file layout
shifts (adding globals, growing games.c, etc.).  Refresh the test
scripts with the new numbers.
"""
import os, struct, sys

BUILD = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                                          'build', 'alcyon')
WANT  = set(sys.argv[1:])

def read_o(path):
    with open(path, 'rb') as f:
        d = f.read()
    magic, tlen, dlen, blen, symlen = struct.unpack('>HIIII', d[:0x12])
    assert magic == 0x601a, f'{path}: bad magic'
    sym_off = 0x1c + tlen + dlen
    return d, tlen, dlen, blen, sym_off, sym_off + symlen

def syms_of(path):
    d, _t, _dd, _b, off, end = read_o(path)
    out = []
    i = off
    while i < end:
        entry = d[i:i+14]
        name = entry[:8].rstrip(b'\0').decode('latin1', 'replace')
        typ, val = struct.unpack('>HI', entry[8:14])
        if (typ & 0x0048) == 0x0048 and i + 28 <= end:
            name += d[i+14:i+28].rstrip(b'\0').decode('latin1', 'replace')
            i += 28
        else:
            i += 14
        out.append((name, typ, val))
    return out

# Same link order alcyon_link.sh uses: gemstart.o, main.o, then the
# alphabetical remainder of the .o directory.
files = sorted(f for f in os.listdir(BUILD) if f.endswith('.o'))
special = ['gemstart.o', 'main.o']
skip    = {'osbind.o', 'crt0.o', 'nofloat.o'}
ordered = special + [f for f in files if f not in special and f not in skip]

with open(os.path.join(BUILD, 'LCP.PRG'), 'rb') as f:
    prg = f.read()
_m, TLEN, DLEN, _BLEN, _SL = struct.unpack('>HIIII', prg[:0x12])

acc_d = acc_b = 0
results = []

for f in ordered:
    _d, tlen, dlen, blen, _o, _e = read_o(os.path.join(BUILD, f))
    for name, typ, val in syms_of(os.path.join(BUILD, f)):
        # 0xa000 mask = defined + globally visible.  Everything else
        # (extern references, local statics) is filtered out.
        if (typ & 0xa000) != 0xa000:
            continue
        seg = typ & 0x0e00
        if   seg == 0x0400:                    # DATA
            base_rel = TLEN + acc_d + val
            section  = 'DATA'
        elif seg == 0x0800:                    # BSS
            base_rel = TLEN + DLEN + acc_b + val
            section  = 'BSS'
        elif seg == 0x0000:                    # TEXT
            base_rel = val
            section  = 'TEXT'
        else:
            continue
        if not WANT or name in WANT:
            results.append((name, section, base_rel, f))
    acc_d += dlen
    acc_b += blen

for name, section, base_rel, f in sorted(results):
    print(f'{name:<20} {section:<4} base+0x{base_rel:05x}  ({f})')
