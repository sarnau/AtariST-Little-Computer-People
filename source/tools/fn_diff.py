#!/usr/bin/env python3
"""fn_diff.py -- side-by-side disassembly of a port function vs its
original-ROM counterpart, for recovering exact C shapes.

Usage:
  python3 source/tools/fn_diff.py NAME [orig_hex_addr]

Locates NAME (port symbol, with or without leading _) in lcp_sym.68k,
finds the candidate site in the reference binary (escalating prefix probe with
relocation + PC-relative wildcards, same rules as verify_bytes.py),
and prints both disassemblies interleaved via m68k-elf-objdump.
Pass orig_hex_addr to pin the original location manually.
"""
import os, re, struct, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from verify_bytes import read_prg, read_syms, pattern, PORT, SYM68K, ORIG


def disasm(blob, vma):
    with tempfile.NamedTemporaryFile(suffix='.bin', delete=False) as f:
        f.write(blob)
        path = f.name
    out = subprocess.run(
        ['m68k-elf-objdump', '-D', '-b', 'binary', '-m', 'm68k',
         f'--adjust-vma=0x{vma:x}', path],
        capture_output=True, text=True).stdout
    os.unlink(path)
    lines = []
    for ln in out.splitlines():
        m = re.match(r'\s*([0-9a-f]+):\s+(\S[\S ]*?)\t(.*)', ln)
        if m:
            lines.append((int(m.group(1), 16), m.group(2).strip(), m.group(3)))
    return lines


def main():
    name = sys.argv[1].lstrip('_')
    pin = int(sys.argv[2], 16) if len(sys.argv) > 2 else None

    ptext, ptsize, prelocs = read_prg(PORT)
    otext, otsize, orelocs = read_prg(ORIG)
    syms = read_syms(SYM68K, ptsize)
    bounds = syms + [(ptsize, '<end>')]

    for k, (off, sname) in enumerate(syms):
        if sname.lstrip('_') == name:
            size = bounds[k + 1][0] - off
            break
    else:
        sys.exit(f'symbol {name} not found')

    code = ptext[off:off + size]
    oo = pin
    if oo is None:
        for probe in (size, 96, 48, 24, 12):
            if probe > size:
                continue
            pat, fixed = pattern(code[:probe], off, prelocs)
            if fixed < 6:
                continue
            hits = [m.start() for m in re.finditer(pat, otext, re.DOTALL)]
            if hits:
                oo = hits[0]
                note = 'FULL MATCH' if probe == size else f'{probe}-byte prefix'
                print(f'# port {sname} @0x{off:05x} len={size}; '
                      f'orig candidate 0x{oo:05x} ({note}, {len(hits)} hits)')
                break
        if oo is None:
            sys.exit('no candidate found; pass an orig address explicitly')

    osize = size + 32
    pl = disasm(code, off)
    ol = disasm(otext[oo:oo + osize], oo)
    W = 44
    print(f'{"PORT":<{W}}| ORIG')
    for i in range(max(len(pl), len(ol))):
        lp = f'{pl[i][0]:05x}: {pl[i][2]}' if i < len(pl) else ''
        lo = f'{ol[i][0]:05x}: {ol[i][2]}' if i < len(ol) else ''
        mark = ' ' if i < len(pl) and i < len(ol) and pl[i][2].split()[0] == ol[i][2].split()[0] else '*'
        print(f'{lp:<{W}}{mark} {lo}')


if __name__ == '__main__':
    main()
