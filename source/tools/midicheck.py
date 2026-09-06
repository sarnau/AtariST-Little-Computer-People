"""Independent sanity check of generated MIDI files: re-parse and report.

Deliberately does not import sngdump -- it re-reads the bytes on disk so a
bug in the writer cannot hide behind the same code.
"""
import struct, sys, os

def varlen(d, i):
    n = 0
    while True:
        b = d[i]; i += 1
        n = (n << 7) | (b & 0x7F)
        if not b & 0x80:
            return n, i

def check(path):
    d = open(path, 'rb').read()
    assert d[:4] == b'MThd', path
    _ln, fmt, ntrk, div = struct.unpack('>IHHH', d[4:14])
    i = 14
    usec = 500000
    notes = unmatched = hanging = 0
    maxpoly = 0
    chans, progs, names = set(), {}, []
    longest = 0
    for _t in range(ntrk):
        assert d[i:i+4] == b'MTrk', f'{path}: track {_t} header'
        tlen = struct.unpack('>I', d[i+4:i+8])[0]
        end = i + 8 + tlen
        i += 8
        t = 0; running = None; on = {}
        while i < end:
            dt, i = varlen(d, i)
            t += dt
            b = d[i]
            if b == 0xFF:
                meta = d[i+1]; ln, i = varlen(d, i+2)
                body = d[i:i+ln]; i += ln
                if meta == 0x51:
                    usec = int.from_bytes(body, 'big')
                elif meta == 0x03:
                    names.append(body.decode('latin1'))
                continue
            if b in (0xF0, 0xF7):
                ln, i = varlen(d, i+1); i += ln
                continue
            if b & 0x80:
                running = b; i += 1
            hi, ch = running & 0xF0, running & 0x0F
            if hi in (0x80, 0x90):
                n, v = d[i], d[i+1]; i += 2
                chans.add(ch)
                if hi == 0x90 and v:
                    on[(ch, n)] = on.get((ch, n), 0) + 1
                    notes += 1
                    maxpoly = max(maxpoly, sum(on.values()))
                elif on.get((ch, n)):
                    on[(ch, n)] -= 1
                else:
                    unmatched += 1
            elif hi == 0xC0:
                progs[ch] = d[i]; i += 1; chans.add(ch)
            elif hi in (0xA0, 0xB0, 0xE0):
                i += 2
            elif hi == 0xD0:
                i += 1
            else:
                raise AssertionError(f'{path}: bad status {running:#04x}')
        assert i == end, f'{path}: track {_t} overran'
        hanging += sum(on.values())
        longest = max(longest, t)
    assert i == len(d), f'{path}: {len(d) - i} trailing bytes'
    secs = longest / div * (usec / 1e6)
    status = 'ok' if not hanging and not unmatched else 'PROBLEM'
    print(f'{os.path.basename(path):<14} fmt{fmt} {ntrk:2d}trk div{div:<4} '
          f'{notes:5d} notes  poly {maxpoly:2d}  ch {sorted(chans)}  '
          f'prog {dict(sorted(progs.items()))}  {secs:6.1f}s  '
          f'hang {hanging} stray {unmatched}  {status}')
    return names

if __name__ == '__main__':
    for p in sorted(sys.argv[1:]):
        check(p)
