#!/usr/bin/env python3
"""gen_ghidra_verify.py -- build the Ghidra name-verification list FROM THE REPO.

WHY THIS EXISTS.  LcpVerifyNames.java used to read a hand-written
`~/ghidra_scripts/lcp_verify.tsv` that was not version-controlled and was
edited by hand during a sync session.  So renaming a port symbol made
`sync_ghidra_names.sh verify` report a FALSE mismatch until someone
remembered to edit that file too -- renaming dg_petok to pat_ok produced
exactly that (`want=dg_petok got=pat_ok`), with Ghidra being the CORRECT
side.  An expectation file that has to be maintained by hand alongside the
thing it checks will drift, and a checker that cries wolf gets ignored.

This generates the list instead, from the same two sources the port's own
build and remap already trust:

  * `build/alcyon/lcp_sym.68k` -- the SYMBOLS link.  Gives every symbol's
    segment and its address.  TEXT and DATA are byte-identical to
    DATA/LCP_STX.PRG, so their link addresses ARE the reference's.
  * `tools/stx_bss_layout.tsv` -- the checked-in BSS spec.  lcp_sym.68k's
    BSS addresses are lo68's, NOT the reference's; the remap moves every
    one of them, so the spec is the only correct source here.

Ghidra address = link address + 0x10000.

TWO THINGS THE SYMBOL TABLE CANNOT GIVE, and how each is handled:

  * Names are TRUNCATED to 8 characters of linkage name, i.e. 7 source
    characters after the leading `_`.  Pushing those into Ghidra is how an
    earlier sync produced `lcp_pat` for `lcp_path`.  They are expanded here
    against the names the port declares at FILE SCOPE, and an expansion is
    accepted only when it is UNIQUE.  Two things have to be kept out of
    that name pool or the uniqueness test stops meaning anything:
      - comments and string literals, because English prose supplies
        "controlled" and "controller" for `_control` and comment text
        supplies a third candidate for `_lcp_pat`.  Stripping them takes
        the ambiguous count from four to zero.
      - K&R parameter declarations, which sit at column 0 exactly like a
        file-scope declaration.  `short index;` between sf_sl's header
        and its brace is a LOCAL, and harvesting it made the libc symbol
        `_index` resolve to it -- the file then asserted a name the repo
        never owned, against an address where Ghidra rightly says
        `strchr`.
  * Library symbols are not the port's to name.  Ninety-odd DRI libc and
    GEM-binding symbols (`___pname`, `__afreeb`, `_malloc_`) have no
    declaration in this tree, so this file cannot say what they should be
    called, and Ghidra's own names for them follow no rule we can
    reproduce (it holds `___pname` but `_afreebase`, `_iob` and `_ctype`).
    The hand-written file guessed, and those guesses were 14 of its 15
    standing mismatches -- permanent noise that a real drift could hide
    behind.  They are omitted, and counted in the header instead.

So every row here is a name the REPO owns, and any mismatch the verify
reports is a real disagreement worth reading.

    python3 source/tools/gen_ghidra_verify.py            # -> build/ghidra/
    python3 source/tools/gen_ghidra_verify.py --out -    # stdout

Rows are `TYPE <TAB> 0xADDRESS <TAB> name`, F for text and A for data/bss,
matching what LcpVerifyNames.java reads.
"""

import argparse
import glob
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CSRC = os.path.abspath(os.path.join(HERE, '..'))
GHIDRA_BASE = 0x10000            # Ghidra address = link address + this

SYM_TEXT, SYM_DATA, SYM_BSS = 0xa200, 0xa400, 0xa100

# Comments and string/char literals.  Stripping these before harvesting
# identifiers is what makes the truncation expansion unambiguous.
CMT = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                 re.S)
IDENT = re.compile(r'[A-Za-z_][A-Za-z0-9_]*')

# File-scope shapes in a .c.  A definition is an identifier at column 0
# followed by '('; a declaration is a type at column 0 ending in one of
# [ = ; or a comma.  K&R parameter declarations look exactly like the
# latter and must NOT be harvested -- `short index;` between sf_sl's
# header and its brace is a LOCAL, and taking it made the libc symbol
# `_index` resolve to it and assert a name the repo never owned.
FN_DEFN = re.compile(r'^([A-Za-z_]\w*)\s*\(')
FN_HDR = re.compile(r'^[A-Za-z_][\w \t\*]*\([^;]*\)\s*$')
FILE_DECL = re.compile(r'^[A-Za-z_]\w*[\w \t\*]*?([A-Za-z_]\w*)\s*(?:\[|=|;|,)')


def read_symbols(path):
    """[(linkage_name, type, value)] from a GEMDOS PRG's symbol table."""
    with open(path, 'rb') as fh:
        d = fh.read()
    if len(d) < 28 or struct.unpack('>H', d[:2])[0] != 0x601A:
        sys.exit('%s is not a GEMDOS PRG' % path)
    _, tlen, dlen, _, slen = struct.unpack('>HIIII', d[:18])
    off, end = 0x1C + tlen + dlen, 0x1C + tlen + dlen + slen
    out = []
    while off < end:
        name = d[off:off + 8].rstrip(b'\0').decode('ascii', 'replace')
        typ, val = struct.unpack('>HI', d[off + 8:off + 14])
        off += 14
        out.append((name, typ, val))
    return out


def scan_c(path):
    """File-scope names defined in a .c, skipping K&R parameter lists."""
    with open(path, errors='replace') as fh:
        text = CMT.sub(' ', fh.read())
    out, in_params = set(), False
    for line in text.split('\n'):
        if line[:1] == '{':
            in_params = False
            continue
        if in_params:
            continue
        m = FN_DEFN.match(line)
        if m:
            out.add(m.group(1))
            in_params = True
            continue
        if FN_HDR.match(line):
            in_params = True
            continue
        m = FILE_DECL.match(line)
        if m:
            out.add(m.group(1))
    return out


def declared_identifiers(root):
    """Names the port's own sources declare at file scope.

    Headers and hand-written assembly contribute everything they name --
    a header holds nothing but declarations, and the .s files declare
    their own globals.  A .c contributes only its file-scope shapes, so
    that a local variable cannot stand in for a library symbol.
    """
    ids = set()
    for h in glob.glob(os.path.join(root, 'include', '*.h')):
        with open(h, errors='replace') as fh:
            ids.update(IDENT.findall(CMT.sub(' ', fh.read())))
    for a in glob.glob(os.path.join(root, '*.s')):
        with open(a, errors='replace') as fh:
            ids.update(IDENT.findall(re.sub(r'\*[^\n]*', ' ', fh.read())))
    for pat in ('*.c', os.path.join('parts', '*.c')):
        for c in glob.glob(os.path.join(root, pat)):
            ids |= scan_c(c)
    return ids


def bss_addresses(path):
    """{linkage_name: reference address} from the checked-in BSS spec."""
    rows = {}
    with open(path) as fh:
        for line in fh:
            if line.startswith('#'):
                continue
            c = line.rstrip('\n').split('\t')
            if len(c) != 3:
                continue
            rows.setdefault(c[0], {})[int(c[1], 16)] = int(c[2], 16)
    # Most symbols have an offset-0 row.  scrbufA does not: it is only ever
    # referenced at +511 through the align-up constant (reloc_audit's
    # category E), so its base is derived from whatever offset it does have.
    return dict((name, (offs[0] if 0 in offs else
                        min(a - o for o, a in offs.items())))
                for name, offs in rows.items())


def expand(link_name, ids):
    """Source name for a linkage name, or None if the repo cannot say.

    A name shorter than 8 characters was not truncated, so it only has to
    BE a declared identifier.  A name of exactly 8 may have been cut, so
    every declared identifier with that prefix is a candidate and the
    expansion is taken only when exactly one exists.
    """
    core = link_name[1:] if link_name.startswith('_') else link_name
    if len(link_name) < 8:
        return core if core in ids else None
    cands = [i for i in ids if i.startswith(core)]
    return cands[0] if len(cands) == 1 else None


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--sym', default=os.path.join(CSRC, 'build', 'alcyon',
                                                  'lcp_sym.68k'))
    ap.add_argument('--layout', default=os.path.join(HERE,
                                                     'stx_bss_layout.tsv'))
    ap.add_argument('--out', default=os.path.join(CSRC, 'build', 'ghidra',
                                                  'lcp_verify.tsv'),
                    help='output path, or - for stdout')
    args = ap.parse_args()

    if not os.path.exists(args.sym):
        sys.exit('no %s -- run source/tools/alcyon_link.sh first' % args.sym)

    syms = read_symbols(args.sym)
    ids = declared_identifiers(CSRC)
    bss = bss_addresses(args.layout)

    rows, skipped, nolayout = [], [], []
    for name, typ, val in syms:
        src = expand(name, ids)
        if src is None:
            skipped.append(name)          # library: not the repo's to name
            continue
        if typ == SYM_TEXT:
            rows.append(('F', val + GHIDRA_BASE, src))
        elif typ == SYM_DATA:
            rows.append(('A', val + GHIDRA_BASE, src))
        elif typ == SYM_BSS:
            # lcp_sym.68k's BSS address is lo68's placement, which the
            # remap overwrites; the spec is where the symbol really lands.
            if name not in bss:
                nolayout.append(name)
                continue
            rows.append(('A', bss[name] + GHIDRA_BASE, src))

    seen = {}
    for kind, addr, src in rows:
        seen.setdefault(addr, []).append(src)
    dupes = sorted((a, n) for a, n in seen.items() if len(n) > 1)

    rows.sort(key=lambda r: (r[0], r[1]))
    text = ['# lcp_verify.tsv -- GENERATED by tools/gen_ghidra_verify.py.'
            '  Do not edit.',
            '# Regenerate after any rename; do NOT hand-patch it, that is'
            ' the drift this replaces.',
            '# %d rows the repo owns; %d library symbols omitted (no'
            ' declaration in this tree).' % (len(rows), len(skipped))]
    for name, addr in dupes:
        text.append('# note: 0x%05x carries %d names: %s'
                    % (name, len(addr), ' '.join(addr)))
    for name in nolayout:
        text.append('# note: %s is BSS with no row in stx_bss_layout.tsv'
                    ' -- omitted' % name)
    text += ['%s\t0x%05x\t%s' % r for r in rows]
    body = '\n'.join(text) + '\n'

    if args.out == '-':
        sys.stdout.write(body)
    else:
        d = os.path.dirname(args.out)
        if d and not os.path.isdir(d):
            os.makedirs(d)
        with open(args.out, 'w') as fh:
            fh.write(body)
        print('wrote %s: %d rows, %d library symbols omitted'
              % (args.out, len(rows), len(skipped)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
