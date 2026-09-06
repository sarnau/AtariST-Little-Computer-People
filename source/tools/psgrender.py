#!/usr/bin/env python3
"""
psgrender.py -- render a Music Studio .SNG through the YM2149 path of the
original playback engine.

This is not the MIDI export.  It is a tick-accurate re-implementation of the
sequencer's internal-synthesis path as it exists in AUDIO.PRG / LCP_STX.PRG:

    mq_tick   960 Hz Timer-A ISR; runs psg_upE every 4th tick (240 Hz)
    mq_advs   two-phase sequencer state machine
    mq_pars   event-stream walker (note events, bars, loops)
    mq_qnne   queue Note-On, dispatch
    mq_expN   expire queued notes -> mq_snof -> Note-Off
    mq_dise   PSG channel allocation, voice stealing, period + mixer writes
    psg_upE   software ADSR envelope processor

The output is a YM2149 register-write log with 960 Hz tick timestamps, which is
exact, plus an audio rendering of that log, which involves the documented
approximations in `synth()`.

Usage:
    psgrender.py FILE.SNG [-o OUT.WAV] [--regs OUT.txt[.gz]] [--rate 44100]
                          [--seconds N] [--quiet]
"""

import argparse
import gzip
import os
import struct
import sys
import wave

import numpy as np

from sngdump import SNG, NDT, N_VOICES

# ---------------------------------------------------------------- constants
TICK_HZ = 960.0                 # MFP Timer A: 2457600 / (64 * 40)
PSG_CLOCK = 2_000_000           # YM2149 master clock on the Atari ST
PSG_STEP_HZ = PSG_CLOCK / 16.0  # 125 kHz tone/noise counter clock

ENV_IDLE, ENV_ATTACK, ENV_DECAY, ENV_SUSTAIN, ENV_RELEASE, ENV_FADEOUT = range(6)

# psg_upE's four ROM tables (identical in both binaries)
EVRT = [0, 360, 180, 120, 85, 72, 60, 45, 30, 20, 15, 12, 10, 8, 6, 4]
EVTT = [0, 1, 2, 3, 4, 5, 6, 8, 12, 18, 24, 30, 36, 45, 60, 90]
EVST = [0, 1, 2, 4, 8, 18, 24, 40, 45, 60, 72, 90, 120, 180, 360, 30000]
EVRL = [0, 360, 180, 90, 45, 20, 15, 9, 8, 6, 5, 4, 3, 2, 1, 0]

NOTE_LO, NOTE_HI = 0x24, 0x60   # g_mnlo, g_mnhi

# psg_freq[0] is the address literal mq_dise loads before indexing the table.
# Both binaries link at base 0, so the literal is (text_size + data_offset).
PSG_FREQ_ABS = {
    'AUDIO.PRG':   0x18890,     # Music Studio   -> data+0x35a
    'LCP_STX.PRG': 0x19922,     # Little Computer People -> data+0x246
}
DEFAULT_PSG_FREQ_ABS = 0x18890

# YM2149 16-level amplitude table, normalised.
YM_VOL = np.array([0.0000, 0.0137, 0.0205, 0.0291, 0.0423, 0.0618, 0.0847,
                   0.1369, 0.1691, 0.2647, 0.3527, 0.4499, 0.5704, 0.6873,
                   0.8482, 1.0000])


def find_prg(explicit=None):
    """Locate a program binary to read psg_freq from.  The script is used from
    both the Music Studio disk directory and the LCP repo, so try both."""
    if explicit:
        return explicit
    here = os.path.dirname(os.path.abspath(__file__))
    cands = []
    for root in (os.path.dirname(here), os.path.dirname(os.path.dirname(here)),
                 os.getcwd()):
        cands += [os.path.join(root, 'AUDIO.PRG'),
                  os.path.join(root, 'DATA', 'LCP_STX.PRG')]
    for c in cands:
        if os.path.exists(c):
            return c
    raise SystemExit('no AUDIO.PRG or DATA/LCP_STX.PRG found; pass --prg\n  tried:\n    '
                     + '\n    '.join(cands))


def load_psg_freq(prg_path):
    """Pull the 128-entry period table straight out of the program binary."""
    from prg import PRG
    p = PRG(prg_path)
    abs_ = PSG_FREQ_ABS.get(os.path.basename(prg_path), DEFAULT_PSG_FREQ_ABS)
    off = abs_ - p.tsz
    return list(struct.unpack('>128H', bytes(p.image[p.tsz + off:p.tsz + off + 256])))


# ------------------------------------------------------------------- engine
class Envelope:
    __slots__ = ('phase', 'a_start', 'a_dur', 'a_target', 'd_dur', 'd_target',
                 's_dur', 's_target', 'r_dur', 'max_volume', 'timer',
                 'volume', 'direction')

    def __init__(self):
        self.phase = ENV_IDLE
        self.a_start = self.a_dur = self.a_target = 0
        self.d_dur = self.d_target = self.s_dur = self.s_target = self.r_dur = 0
        self.max_volume = self.timer = self.volume = 0
        self.direction = 1


class Engine:
    """Faithful re-implementation of the PSG playback path."""

    def __init__(self, sng, psg_freq):
        self.s = sng
        self.freq = psg_freq
        d = sng.data
        self.d = d

        # ---- mq_setp / mq_stap
        self.pos = sng.header_end          # mi_sqpos (mq_skip's result)
        self.end = len(d)                  # mi_seqE
        self.spb = sng.spb                 # g_mtspb
        self.tpb = sng.spb                 # mi_tpb
        self.vel = sng.velocity            # mi_vel  (= mi_dvel at song start)
        self.dvel = sng.velocity
        self.cvol = sng.psg_volume         # psg_cvol
        self.dvol = sng.psg_volume
        self.evq = []                      # mi_evq, triples
        self.lstk = []                     # mi_lstk
        self.tick = 0                      # g_mtcou
        self.lpTk = self.nxTk = self.nlp0 = 100
        self.mtpre = self.mtdiv = 100
        self.phase = 1                     # g_mspha = SEQ_PHASE_PARSE
        self.active = True                 # g_msmsa
        self.playing = True                # mi_play
        self.ntAc = False                  # psg_ntAc
        self.scale = sng.scale             # g_mstr
        self.chmap = [0] + list(sng.chanmap)
        self.noSt = [0] * 128              # mi_noSt

        # per-event scratch (mi_* globals unpacked by mq_pars)
        self.ccha = self.cnot = 0
        self.nnOn = self.nnOf = self.lasT = 0
        self.ndur = 0

        # ---- PSG state
        self.chNt = [0, 0, 0, 0]           # psg_chNt (4th slot always 0)
        self.env = [Envelope() for _ in range(3)]
        self.rdel = [0, 0, 0]
        self.racc = [0, 0, 0]
        self.regs = [0] * 14
        self.writes = []                   # (tick, reg, value)
        self.stolen = 0
        self.dropped = 0

    # ------------------------------------------------------------ registers
    def wr(self, reg, val):
        val &= 0xFF
        if self.regs[reg] != val or reg >= 8:
            self.regs[reg] = val
            self.writes.append((self.tick, reg, val))

    # ------------------------------------------------------------ mq_dise
    def dise_note_on(self, note, voice):
        # allocate: first silent channel, else steal the one furthest along
        ch = 0
        while ch < 4 and self.chNt[ch]:
            ch += 1
        if ch == 3:
            best = 0
            for c in (1, 2):
                if self.env[c].phase > self.env[c - 1].phase:
                    best = c
            ch = best
            self.stolen += 1
        if not (NOTE_LO <= note <= NOTE_HI):
            self.dropped += 1
            return

        e = self.env[ch]
        raw = self.s.envelopes[voice - 1].raw
        e.a_start, e.a_dur, e.a_target, e.d_dur = raw[0], raw[1], raw[2], raw[3]
        e.d_target, e.s_dur, e.s_target, e.r_dur = raw[4], raw[5], raw[6], raw[7]

        phase = ENV_ATTACK
        mixer_nibble = (e.a_start >> 4) & 0xF
        e.a_start &= 0xF
        shift = (2 - ((e.a_dur >> 4) & 0xF)) * 12
        e.a_dur &= 0xF

        mixer_bits = mixer_nibble << ch
        noise_mask = (~(9 << ch)) & 0xFFFF

        idx = note + shift
        period = self.freq[idx] if 0 <= idx < 128 else 0
        self.wr(6, (period // 60) & 0xFF)                    # noise period
        self.wr(7, (self.regs[7] & (noise_mask | 0xC0)) | mixer_bits)

        if idx > 22:
            self.wr(ch * 2, period & 0xFF)
            self.wr(ch * 2 + 1, (period >> 8) & 0x0F)
        else:
            phase = ENV_FADEOUT

        self.chNt[ch] = note
        if phase == ENV_FADEOUT:
            e.volume = 0
        e.max_volume = self.cvol
        e.timer = 1
        e.phase = phase
        self.ntAc = True

    def dise_note_off(self, note):
        ch = 0
        while ch < 4 and self.chNt[ch] != note:
            ch += 1
        if ch >= 3:
            return
        self.chNt[ch] = 0
        self.env[ch].phase = ENV_RELEASE
        self.env[ch].timer = 0

    # ------------------------------------------------------------ psg_upE
    def upE(self):
        for i in range(3):
            e = self.env[i]
            if not e.phase:
                continue
            ph = e.phase
            while True:
                if ph == ENV_ATTACK:
                    e.volume = e.a_start
                    e.phase = ENV_DECAY
                    if not e.a_dur:
                        e.volume = e.a_target
                        e.timer = 0
                        ph = ENV_DECAY
                        continue
                    e.timer = e.a_dur
                    if e.a_start > e.a_target:
                        self.rdel[i] = e.a_start - e.a_target
                        e.direction = -1
                    else:
                        e.direction = 1
                        self.rdel[i] = e.a_target - e.a_start
                    self.rdel[i] *= EVRT[e.timer & 15]
                    e.timer = EVTT[e.timer & 15]
                    self.racc[i] = 0
                elif ph == ENV_DECAY:
                    if e.timer > 0:
                        e.timer -= 1
                        self._ramp(i)
                    else:
                        e.timer -= 1
                        if not e.d_dur:
                            e.volume = e.d_target
                            e.timer = 0
                            ph = ENV_SUSTAIN
                            continue
                        e.phase = ENV_SUSTAIN
                        e.timer = e.d_dur
                        if e.a_target > e.d_target:
                            self.rdel[i] = e.a_target - e.d_target
                            e.direction = -1
                        else:
                            e.direction = 1
                            self.rdel[i] = e.d_target - e.a_target
                        self.rdel[i] *= EVRT[e.timer & 15]
                        e.timer = EVTT[e.timer & 15]
                        self.racc[i] = 0
                elif ph == ENV_SUSTAIN:
                    if e.timer > 0:
                        e.timer -= 1
                        self._ramp(i)
                    else:
                        e.timer -= 1
                        if not e.s_dur:
                            e.volume = e.s_target
                            e.timer = 0
                            ph = ENV_RELEASE
                            continue
                        e.phase = ENV_RELEASE
                        e.timer = EVST[e.s_dur & 15]
                        if e.d_target > e.s_target:
                            self.rdel[i] = e.d_target - e.s_target
                            e.direction = -1
                        else:
                            e.direction = 1
                            self.rdel[i] = e.s_target - e.d_target
                        self.rdel[i] *= EVRL[e.s_dur & 15]
                        self.racc[i] = 0
                elif ph == ENV_RELEASE:
                    if e.timer > 0:
                        e.timer -= 1
                        self._ramp(i)
                    else:
                        e.timer -= 1
                        if e.r_dur:
                            e.phase = ENV_FADEOUT
                            e.timer = e.r_dur
                            self.rdel[i] = e.volume * EVRT[e.timer & 15]
                            e.direction = -1
                            e.timer = EVTT[e.timer & 15]
                            self.racc[i] = 0
                        else:
                            e.timer = 0
                            ph = ENV_FADEOUT
                            continue
                elif ph == ENV_FADEOUT:
                    if e.timer > 0 and e.volume:
                        e.timer -= 1
                        self._ramp(i)
                    else:
                        e.timer -= 1
                        e.volume = 0
                        e.phase = ENV_IDLE
                break
            out = min(e.volume, e.max_volume)
            self.wr(8 + i, max(0, out) & 0x0F)

    def _ramp(self, i):
        e = self.env[i]
        self.racc[i] += self.rdel[i]
        while self.racc[i] > 0x168:
            e.volume += e.direction
            self.racc[i] -= 0x168

    # ------------------------------------------------------------ mq_snof
    def snof(self, k):
        if self.evq[k + 1] & 0x80:
            return
        note = self.evq[k + 1] & 0xFF
        if note > NOTE_HI or note < NOTE_LO or note == 0:
            return
        self.dise_note_off(note)

    def expN(self, val):
        i = 0
        while i < len(self.evq):
            self.evq[i] -= val
            if self.evq[i] <= 0:
                self.snof(i)
                del self.evq[i:i + 3]
                continue
            i += 3

    # ------------------------------------------------------------ mq_qnne
    def qnne(self):
        if len(self.evq) < 58 * 3:
            self.evq.append(self.ndur)
            self.evq.append(((self.lasT << 1) | self.cnot) if self.nnOn else 0)
            self.evq.append(self.chmap[self.ccha])
        else:
            return
        if self.cnot > NOTE_HI or self.cnot < NOTE_LO:
            return
        if self.nnOf:
            self.noSt[self.cnot] = 0
        if self.lasT:
            self.noSt[self.cnot] = self.ccha
        if self.nnOf:
            return
        self.dise_note_on(self.cnot, self.ccha)

    # ------------------------------------------------------------ mq_rdur
    def rdur(self):
        d, n = self.d, self.end
        while self.pos < n and d[self.pos] == 0:
            self.pos += 1
        if self.pos + 1 < n and not d[self.pos] & 0x80:
            self.nlp0 = (NDT[d[self.pos + 1] & 0x1F] - 1) * self.spb
        else:
            self.nlp0 = 0

    # ------------------------------------------------------------ mq_pars
    def pars(self):
        d, n = self.d, self.end
        if self.pos >= n or d[self.pos] != 0:
            return 0
        self.pos += 1
        if self.pos >= n:
            return 0
        evTf = 0
        self.rdur()
        if self.pos >= n:
            return 0

        while self.pos < n and d[self.pos] != 0:
            b = d[self.pos]
            if not b & 0x80:
                if self.pos + 2 >= n:
                    return 0
                evTf = 1
                b0, b1, b2 = d[self.pos], d[self.pos + 1], d[self.pos + 2]
                self.nnOn = 16 - (b0 & 0x10)
                self.lasT = b0 & 0x40
                self.nnOf = b0 & 0x20
                self.ccha = b0 & 0x0F
                if b1 & 0x20:
                    self.vel, self.cvol = 0x7F, 0x0F
                else:
                    self.vel, self.cvol = self.dvel, self.dvol
                mode = b1 & 0xC0
                self.ndur = (NDT[b1 & 0x1F] - 1) * self.spb
                raw = b2 & 0x7F
                if mode:
                    self.cnot = raw
                    if mode & 0x80:
                        self.cnot += -1 if mode & 0x40 else 1
                else:
                    self.cnot = self.scale[raw] if raw < len(self.scale) else raw
                self.pos += 3
                if self.nnOn:
                    self.qnne()
            else:
                self.pos += 1
                if b == 0x82:
                    if not evTf:
                        self.rdur()
                        if self.pos >= n:
                            return 0
                elif b == 0x85:
                    count = d[self.pos]
                    if len(self.lstk) < 20:
                        self.lstk.append([self.pos + 1, count - 1])
                    self.pos += 1
                    self.rdur()
                    if self.pos >= n:
                        return 0
                elif b == 0x86:
                    if self.lstk:
                        if self.lstk[-1][1] == 0:
                            self.lstk.pop()
                        else:
                            self.lstk[-1][1] -= 1
                            self.pos = self.lstk[-1][0]
                    self.rdur()
                    if self.pos >= n:
                        return 0
                elif b == 0xFF:
                    return 0
        return 1

    # ------------------------------------------------------------ mq_advs
    def advs(self):
        if self.phase == 0:
            self.expN(self.tick - self.lpTk)
            self.lpTk = self.tick
            self.mtpre = self.tpb
            self.phase = 1
            self.nxTk += self.tpb
        elif self.phase == 1:
            self.phase = 0
            if self.pars():
                self.nxTk += self.nlp0
                self.nlp0 = self.nxTk - self.tick
                if self.nlp0 > 0:
                    self.mtpre = self.nlp0
            else:
                self.phase = 2
                self.mtpre = self.tpb
                self.nxTk += self.mtpre
        else:
            self.expN(self.tick - self.lpTk)
            self.lpTk = self.tick
            self.mtpre = self.tpb
            self.nxTk += self.tpb
            if not self.evq:
                for e in self.env:
                    e.phase = ENV_IDLE
                self.playing = self.ntAc = self.active = False
                self.wr(8, 0)
                self.wr(9, 0)
                self.wr(10, 0)

    # ------------------------------------------------------------ mq_tick
    def run(self, max_ticks=960 * 60 * 30):
        while self.tick < max_ticks:
            self.tick += 1
            if self.active:
                self.mtpre -= 1
                self.mtdiv -= 1
                if self.mtdiv == 0:
                    self.mtdiv = 4
                    self.upE()
                elif self.mtpre <= 0:
                    self.advs()
            elif self.ntAc:
                self.mtdiv -= 1
                if self.mtdiv == 0:
                    self.mtdiv = 4
                    self.upE()
            else:
                break
        return self.writes


# -------------------------------------------------------------------- synth
def _lfsr_bits(n=(1 << 17) - 1):
    """YM2149 noise: 17-bit LFSR, output bit 0, feedback bit0 ^ bit3."""
    bits = np.empty(n, dtype=np.float64)
    r = 1
    for i in range(n):
        bits[i] = r & 1
        r = (r >> 1) | (((r ^ (r >> 3)) & 1) << 16)
    return bits


_NOISE = None


def noise_tables():
    global _NOISE
    if _NOISE is None:
        b = _lfsr_bits()
        _NOISE = (b, np.concatenate(([0.0], np.cumsum(b))), len(b), b.sum())
    return _NOISE


def _noise_int(x, ncum, bits, nlen, ntot):
    """Exact integral of the periodic LFSR bit stream up to each x (in LFSR
    steps).  Interpolating inside the sample the boundary falls in matters:
    when the noise clock is slower than the output rate, flooring the index
    turns the average into a nearest-neighbour sample and produces spikes."""
    q, r = np.divmod(x, float(nlen))
    i = r.astype(np.int64)
    frac = r - i
    return q * ntot + ncum[i] + frac * bits[i]


def _square_avg(x0, step, n, period):
    """Mean of a 50 % square wave over n consecutive sample windows.

    x0 is the phase in 125 kHz counter steps, `step` the window width in the
    same units, `period` the YM tone period (the output toggles every
    `period` counter steps).  Averaging the exact waveform over each window is
    the box filter an ideal resampler would apply, so this is anti-aliased
    rather than point-sampled.
    """
    p = max(int(period), 1)
    full = 2.0 * p
    edges = x0 + step * np.arange(n + 1, dtype=np.float64)
    q, r = np.divmod(edges, full)
    high = q * p + np.minimum(r, p)
    return np.diff(high) / step


def synth(writes, rate=44100, tail_ticks=0):
    """Render a register-write log to a mono float array.

    Exact: register values, their 960 Hz timing, tone periods, mixer routing
    and the 16-level amplitude table.
    Approximated: the tone/noise AND in the YM's output stage is computed as a
    product of the two box-filtered duty signals (treats them as independent),
    and the channels are summed linearly rather than through the ST's analogue
    mixing.
    """
    if not writes:
        return np.zeros(0, dtype=np.float64)
    noise, ncum, nlen, ntot = noise_tables()

    end_tick = writes[-1][0] + tail_ticks
    total = int(end_tick / TICK_HZ * rate) + 1
    out = np.zeros(total, dtype=np.float64)

    regs = [0] * 14
    phase = [0.0, 0.0, 0.0]          # tone counter phase, 125 kHz steps
    nphase = 0.0                     # noise counter phase, in LFSR steps
    step = PSG_STEP_HZ / rate        # counter steps per output sample

    i = 0
    cur = 0                          # current output sample index
    n_writes = len(writes)
    while i < n_writes:
        t = writes[i][0]
        while i < n_writes and writes[i][0] == t:
            _, reg, val = writes[i]
            regs[reg] = val
            i += 1
        nxt = writes[i][0] if i < n_writes else end_tick
        start = cur
        stop = min(total, int(nxt / TICK_HZ * rate))
        n = stop - start
        if n <= 0:
            continue

        mix = regs[7]
        np_ = max(regs[6] & 0x1F, 1)
        nstep = step / np_
        seg = np.zeros(n, dtype=np.float64)
        for c in range(3):
            vol = YM_VOL[regs[8 + c] & 0x0F]
            period = (regs[c * 2] | ((regs[c * 2 + 1] & 0x0F) << 8))
            tone_off = (mix >> c) & 1
            noise_off = (mix >> (c + 3)) & 1
            sig = _square_avg(phase[c], step, n, period) if not tone_off \
                else np.ones(n)
            phase[c] += step * n
            if not noise_off:
                e = nphase + nstep * np.arange(n + 1, dtype=np.float64)
                sig = sig * (np.diff(_noise_int(e, ncum, noise, nlen, ntot))
                             / max(nstep, 1e-12))
            if vol:
                seg += sig * vol
        nphase += nstep * n
        out[start:stop] = seg
        cur = stop

    # The YM's output is unipolar; the ST couples it through a capacitor, so
    # strip the volume-dependent DC step the same way instead of leaving a
    # thump on every envelope change.  A centred moving-average subtraction is
    # used rather than a one-pole IIR: it is linear-phase, needs no recursion,
    # and is numerically exact in float64.
    out /= 3.0                      # unit scale: three channels at full volume
    return dc_block(out, rate)


def dc_block(x, rate, cutoff=20.0):
    """High-pass by subtracting a centred moving average (first null = cutoff)."""
    n = max(int(rate / cutoff) | 1, 3)
    if len(x) < n * 2:
        return x - x.mean() if len(x) else x
    half = n // 2
    pad = np.concatenate((np.full(half, x[0]), x, np.full(half, x[-1])))
    c = np.concatenate(([0.0], np.cumsum(pad)))
    avg = (c[n:] - c[:-n]) / n
    return x - avg[:len(x)]


def write_wav(path, samples, rate, gain=1.0):
    pcm = np.clip(samples * gain * 32767.0, -32768, 32767).astype('<i2')
    with wave.open(path, 'wb') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(pcm.tobytes())


def write_regs(path, writes, sng):
    op = gzip.open if path.endswith('.gz') else open
    with op(path, 'wt') as f:
        f.write(f'# YM2149 register writes for {sng.name}\n')
        f.write(f'# tick rate {TICK_HZ:g} Hz, PSG clock {PSG_CLOCK} Hz, '
                f'tempo {sng.tempo} bpm, ticks per 1/24 note {sng.spb}\n')
        f.write('# tick\treg\tvalue\n')
        for t, r, v in writes:
            f.write(f'{t}\t{r}\t{v}\n')


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('song')
    ap.add_argument('-o', '--out', help='output WAV (default: alongside input)')
    ap.add_argument('--regs', help='also write the register log here (.gz ok)')
    ap.add_argument('--rate', type=int, default=44100)
    ap.add_argument('--seconds', type=float, default=600.0,
                    help='safety cap on rendered length')
    ap.add_argument('--prg', default=None,
                    help='AUDIO.PRG to read psg_freq from')
    ap.add_argument('--gain', type=float, default=None,
                    help='output gain; default normalises this file alone')
    ap.add_argument('--quiet', action='store_true')
    a = ap.parse_args()

    freq = load_psg_freq(find_prg(a.prg))

    sng = SNG(open(a.song, 'rb').read(), os.path.basename(a.song))
    eng = Engine(sng, freq)
    writes = eng.run(int(a.seconds * TICK_HZ))
    samples = synth(writes, a.rate, tail_ticks=int(0.25 * TICK_HZ))

    peak = float(np.abs(samples).max()) if len(samples) else 0.0
    gain = a.gain if a.gain is not None else (0.89 / peak if peak else 1.0)
    out = a.out or os.path.splitext(a.song)[0] + '.wav'
    write_wav(out, samples, a.rate, gain)
    if a.regs:
        write_regs(a.regs, writes, sng)
    if not a.quiet:
        print(f'{os.path.basename(a.song):<14} -> {os.path.basename(out):<14} '
              f'{len(samples) / a.rate:6.1f}s  {len(writes):6d} register writes  '
              f'{eng.stolen:4d} voice steals  {eng.dropped:3d} out-of-range  '
              f'peak {peak:.3f} gain {gain:.2f}')


if __name__ == '__main__':
    main()
