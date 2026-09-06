#!/usr/bin/env python3
"""xbin_diff.py -- find code LCP_STX shares with another Atari ST binary.

Two Activision ST titles built with the same toolchain will share their
DRI library, and may share a great deal more.  Diffing LCP_STX against
THE MUSIC STUDIO (2026-09-06) established that cp_main is not LCP's
code at all: 7289 of cp_asm's 7499 bytes are byte-identical to Music
Studio's AUDIO.PRG, which corroborates from an unrelated binary that
the region is hand assembly rather than compiled C.  The same run found
1043 shared bytes in the MIDI sequencer -- the player lineage the
shared .SNG format had only implied -- and ZERO in sf_irqp, which is
what closed Music Studio as a source of evidence about g_sfDoB.

That comparison was worth keeping, hence this.

    python3 source/tools/xbin_diff.py OTHER.PRG [MORE.PRG ...]
    python3 source/tools/xbin_diff.py DISK.STX        # extracted first

Options:
    --ref PATH     reference binary (default $LCP_REF or DATA/LCP_STX.PRG)
    --window N     minimum match seed, bytes (default 32)
    --top N        how many individual runs to list (default 12)
    --min N        only report per-function coverage at or above N bytes

WHAT THE NUMBERS MEAN, and the one trap:

  * Relocated longwords hold ABSOLUTE addresses, and two binaries never
    agree on those.  So a shared function does NOT appear as one run --
    it appears as several, broken at every relocation site.  Judge by
    the per-function COVERAGE percentage, never by the longest run.
  * A run is reported at its position in the REFERENCE, and attributed
    to the function it lands in using lcp_sym.68k's text symbols, so
    the attribution follows the build instead of a hardcoded map.  Run
    a link first if lcp_sym.68k is missing.
  * DRI library matches are expected and uninteresting: everything from
    vswr_mode (0x1733a) up is library in LCP_STX.  Matches BELOW that
    are the finding.
"""

import os
import re
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
SYM = os.path.join(HERE, '..', 'build', 'alcyon', 'lcp_sym.68k')
LIB_START = 0x1733a          # vswr_mode: everything above is DRI library


def prg_text(path):
    """TEXT segment of a GEMDOS PRG, plus its size."""
    d = open(path, 'rb').read()
    if len(d) < 28 or struct.unpack('>H', d[:2])[0] != 0x601A:
        return None, 0
    tlen = struct.unpack('>I', d[2:6])[0]
    return d[0x1C:0x1C + tlen], tlen


def text_symbols(path, tsize):
    """[(text_offset, name)] for the reference, sorted."""
    try:
        d = open(path, 'rb').read()
    except IOError:
        return []
    magic, t, dd, b, s = struct.unpack('>HIIII', d[:18])
    if magic != 0x601A or t != tsize:
        return []                      # a stale link: attribute nothing
    off, end = 0x1C + t + dd, 0x1C + t + dd + s
    syms = {}
    while off < end:
        name = d[off:off + 8].rstrip(b'\0').decode('ascii', 'replace')
        typ, val = struct.unpack('>HI', d[off + 8:off + 14])
        off += 14
        if typ & 0x0200 and val < t:
            syms.setdefault(val, name)
    return sorted(syms.items())


def shared_runs(ref, other, window):
    """Maximal byte-identical runs, as [(length, ref_off, other_off)]."""
    index = {}
    for i in range(len(ref) - window):
        index.setdefault(ref[i:i + window], i)
    runs, i = [], 0
    while i < len(other) - window:
        j = index.get(other[i:i + window])
        if j is None:
            i += 1
            continue
        n = window
        while (i + n < len(other) and j + n < len(ref)
               and other[i + n] == ref[j + n]):
            n += 1
        runs.append((n, j, i))
        i += n
    return runs


def extract_stx(path):
    """Run the project's own extractor; return the .PRG files it wrote."""
    out = tempfile.mkdtemp(prefix='xbin_')
    tool = os.path.join(HERE, 'stx_extract.py')
    try:
        subprocess.run([sys.executable, tool, path, out],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                       check=True)
    except (subprocess.CalledProcessError, OSError) as exc:
        print('  cannot extract %s: %s' % (path, exc), file=sys.stderr)
        return []
    found = [os.path.join(out, f) for f in sorted(os.listdir(out))
             if f.upper().endswith(('.PRG', '.TOS', '.APP'))]
    if not found:
        print('  %s: no executable inside' % os.path.basename(path),
              file=sys.stderr)
    return found


def report(ref, refsize, syms, path, window, top, minbytes):
    other, osize = prg_text(path)
    if other is None:
        print('  %s is not a GEMDOS PRG -- skipped' % os.path.basename(path))
        return
    runs = shared_runs(ref, other, window)
    cov = bytearray(refsize)
    for n, j, _ in runs:
        for k in range(j, j + n):
            cov[k] = 1
    total = sum(cov)
    print('\n=== %s ===' % os.path.basename(path))
    print('  text %d bytes; shared with the reference: %d (%.2f%% of it)'
          % (osize, total, 100.0 * total / osize if osize else 0.0))
    below = sum(cov[:LIB_START])
    print('  of that, BELOW the library boundary 0x%05x: %d bytes%s'
          % (LIB_START, below, '' if below else '  (library only -- '
             'nothing but the shared DRI runtime)'))
    if not total:
        return

    # Per-function coverage: the number that actually means something,
    # because relocations chop a shared function into several runs.
    bounds = syms + [(refsize, '<end>')]
    rows = []
    for k, (off, name) in enumerate(syms):
        end = bounds[k + 1][0]
        got = sum(cov[off:end])
        if got >= minbytes:
            rows.append((got, end - off, off, name))
    rows.sort(reverse=True)
    if rows:
        print('\n  shared by function (reference symbols):')
        print('    %-12s %8s %8s  %s' % ('function', 'shared', 'size', 'cover'))
        for got, size, off, name in rows[:top]:
            mark = '   <-- library' if off >= LIB_START else ''
            print('    %-12s %8d %8d  %5.1f%%  @0x%05x%s'
                  % (name, got, size, 100.0 * got / size if size else 0.0,
                     off, mark))

    runs.sort(reverse=True)
    print('\n  longest individual runs (a shared function splits at every'
          ' relocation):')
    for n, j, i in runs[:top]:
        who = '?'
        for off, name in syms:
            if off <= j:
                who = name
            else:
                break
        print('    %6d bytes  ref 0x%05x (%s)  <-> other 0x%05x'
              % (n, j, who, i))


def main():
    args = sys.argv[1:]
    ref_path = os.environ.get('LCP_REF',
                              os.path.join(ROOT, 'DATA', 'LCP_STX.PRG'))
    window, top, minbytes = 32, 12, 16
    targets = []
    it = iter(range(len(args)))
    i = 0
    while i < len(args):
        a = args[i]
        if a == '--ref':
            i += 1; ref_path = args[i]
        elif a == '--window':
            i += 1; window = int(args[i])
        elif a == '--top':
            i += 1; top = int(args[i])
        elif a == '--min':
            i += 1; minbytes = int(args[i])
        elif a in ('-h', '--help'):
            print(__doc__)
            return 0
        else:
            targets.append(a)
        i += 1

    if not targets:
        print(__doc__)
        return 2

    ref, refsize = prg_text(ref_path)
    if ref is None:
        print('reference %s is not a GEMDOS PRG' % ref_path, file=sys.stderr)
        return 2
    syms = text_symbols(SYM, refsize)
    print('reference: %s (text %d bytes)' % (ref_path, refsize))
    if syms:
        print('symbols  : %d text symbols from lcp_sym.68k' % len(syms))
    else:
        print('symbols  : none usable -- runs will not be attributed.'
              '  Link first, or the reference is not the current build.')

    for t in targets:
        if t.lower().endswith('.stx'):
            for prg in extract_stx(t):
                report(ref, refsize, syms, prg, window, top, minbytes)
        else:
            report(ref, refsize, syms, t, window, top, minbytes)
    return 0


if __name__ == '__main__':
    sys.exit(main())
