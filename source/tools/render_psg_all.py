#!/usr/bin/env python3
"""Render every .SNG through the PSG engine with one common output gain, so
relative loudness between songs is preserved.  Two passes: measure, then write.

    cd analysis && python3 render_psg_all.py [SRC_DIR] [DST_DIR]
"""
import glob, os, sys
import numpy as np
from sngdump import SNG
from psgrender import (Engine, synth, write_wav, write_regs, load_psg_freq,
                       find_prg, TICK_HZ)

SRC = sys.argv[1] if len(sys.argv) > 1 else '..'
DST = sys.argv[2] if len(sys.argv) > 2 else '../psg'
PRG = sys.argv[3] if len(sys.argv) > 3 else find_prg()
RATE = 44100
HEADROOM = 0.89

# psg_freq lives at a different address in each binary; load_psg_freq resolves
# it from the literal that binary's mq_dise uses.
freq = load_psg_freq(PRG)
songs = sorted(glob.glob(os.path.join(SRC, '*.SNG')) +
               glob.glob(os.path.join(SRC, '*.ORG')))
os.makedirs(DST, exist_ok=True)

def render(path):
    sng = SNG(open(path, 'rb').read(), os.path.basename(path))
    eng = Engine(sng, freq)
    writes = eng.run(int(600 * TICK_HZ))
    return sng, eng, writes, synth(writes, RATE, tail_ticks=int(0.25 * TICK_HZ))

peaks = {}
for p in songs:
    _s, _e, _w, y = render(p)
    peaks[p] = float(np.abs(y).max()) if len(y) else 0.0
loudest = max(peaks, key=peaks.get)
gain = HEADROOM / peaks[loudest]
print(f'common gain {gain:.3f} (loudest raw peak {peaks[loudest]:.4f} in '
      f'{os.path.basename(loudest)})\n')

print(f"{'song':<14} {'secs':>6} {'writes':>7} {'steals':>7} {'peak':>6}")
for p in songs:
    sng, eng, writes, y = render(p)
    base = os.path.splitext(os.path.basename(p))[0]
    write_wav(os.path.join(DST, base + '.wav'), y, RATE, gain)
    write_regs(os.path.join(DST, base + '.regs.txt.gz'), writes, sng)
    print(f'{base:<14} {len(y)/RATE:6.1f} {len(writes):7d} {eng.stolen:7d} '
          f'{peaks[p]*gain:6.3f}')
