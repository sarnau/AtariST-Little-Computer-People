#!/usr/bin/env python3
"""
sngdump.py -- decoder for Activision "The Music Studio" .SNG / .ORG / .SND files
(Atari ST, 1985).

The format is documented in ../SNG_FORMAT.md.  Everything implemented here is
taken from the playback engine itself: AUDIO.PRG (Music Studio) and LCP_STX.PRG
(Little Computer People), which share the same sequencer object code.

Usage:
    sngdump.py FILE...                 structure report
    sngdump.py --events FILE           + full event-stream listing
    sngdump.py --midi OUT.MID FILE     export a standard MIDI file
    sngdump.py --json FILE             machine-readable dump
"""

import argparse
import json
import struct
import sys

MAGIC = b'\xcdMstudio\xcd'

TYPE_SOUND = 1          # .SND -- instrument set only
TYPE_SONG = 2           # .SNG / .ORG -- instrument set + event stream

# ---------------------------------------------------------------- offsets
OFF_NAMES_A = 0x00A     # 15 x 10 bytes
OFF_ENVELOPES = 0x0A0   # 15 x  8 bytes
OFF_NAMES_B = 0x118     # 15 x 10 bytes
OFF_CHANMAP = 0x1AE     # 15 bytes
OFF_PROGMAP = 0x1BD     # 15 bytes
OFF_PTRS = 0x1CC        # 5 longwords (lyric pointers / editor scratch)
OFF_TITLE = 0x1E0       # 32 bytes
OFF_TRAILER = 0x200     # 8 bytes
OFF_STREAM = 0x208      # mi_dbase
N_VOICES = 15

# ------------------------------------------------- tables lifted from ROM
# mi_ndt -- note duration table, indexed by bits 0..4 of a note event's
# byte 1.  Unit = 1/24 quarter note.
NDT = [0, 2, 2, 3, 4, 5, 6, 8, 9, 12, 16, 18, 24, 32, 36, 48, 64, 72, 96, 128,
       144, 0]

# g_msmk -- key-signature chord masks, indexed by the operand of header
# command 0x80.  Bit n clear => degree DEGREE[n] is displaced by one semitone
# (up for index <= 8, down for index > 8).
MSMK = [0xFF, 0xFF, 0x77, 0x37, 0x33, 0x13, 0x11, 0x01,
        0x00, 0xFE, 0xEE, 0xEC, 0xCC, 0xC8, 0x88, 0x00]
DEGREE = [11, 9, 7, 5, 4, 2, 0]     # bit 0 -> B, bit 1 -> A, ... bit 6 -> C

KEY_NAMES = ['C', 'C', 'G', 'D', 'A', 'E', 'B', 'F#', 'C#',
             'F', 'Bb', 'Eb', 'Ab', 'Db', 'Gb', 'Cb']

NOTE_NAMES = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']

# Nominal musical value of each mi_ndt entry (24 = quarter note).
DUR_NAMES = {
    2: '1/64', 3: '1/32', 4: '1/16T', 5: '1/32.', 6: '1/16', 8: '1/8T',
    9: '1/16.', 12: '1/8', 16: '1/4T', 18: '1/8.', 24: '1/4', 32: '1/2T',
    36: '1/4.', 48: '1/2', 64: '1/1T', 72: '1/2.', 96: '1/1', 128: '2/1T',
    144: '1/1.',
}

TICK_HZ = 2457600.0 / (64 * 0x28)   # Timer A: /64 prescaler, data 0x28 -> 960 Hz

# Mixer nibble (high nibble of the attack_start_vol byte).  mq_dise clears
# both the tone- and noise-disable bits for the channel, then ORs this nibble
# in: bit 0 disables tone, bit 3 disables noise.
MIXER_FLAGS = {0x0: 'tone+noise', 0x1: 'noise', 0x8: 'tone', 0x9: 'silent'}


def note_name(n):
    if n is None or n < 0:
        return '--'
    return f'{NOTE_NAMES[n % 12]}{n // 12 - 1}'


def build_scale_table(key):
    """mq_bust: 132-entry note remap for a key signature (0..15)."""
    tab = list(range(0x84))
    for i in (1, 3, 6, 8, 10):
        tab[i] = -1
    if key == 1:
        return tab
    shift = -1 if key > 8 else 1
    for base in range(0, 0x84, 12):
        mask = MSMK[key]
        for bit in range(7):
            if not (mask >> bit) & 1:
                idx = base + DEGREE[bit]
                if idx < 0x84:
                    tab[idx] += shift
    return tab


def cstr(buf):
    return buf.split(b'\0')[0].decode('latin1')


# --------------------------------------------------------------- structures
class Envelope:
    """One 8-byte ADSR record, as psg_cpE copies it into PSG_ENVELOPE+1."""

    FIELDS = ('attack_start_vol', 'attack_duration', 'attack_target_vol',
              'decay_duration', 'decay_target_vol', 'sustain_duration',
              'sustain_target_vol', 'release_duration')

    def __init__(self, raw):
        self.raw = bytes(raw)
        for name, val in zip(self.FIELDS, raw):
            setattr(self, name, val)
        # mq_dise splits two of the bytes into nibble pairs before use.
        self.mixer = (self.attack_start_vol >> 4) & 0xF
        self.attack_start_vol &= 0xF
        self.octave = 2 - ((self.attack_duration >> 4) & 0xF)
        self.attack_duration &= 0xF

    def describe(self):
        return (f'A {self.attack_start_vol:2d}->{self.attack_target_vol:2d} '
                f'd{self.attack_duration:<2d} '
                f'D ->{self.decay_target_vol:2d} d{self.decay_duration:<2d} '
                f'S ->{self.sustain_target_vol:2d} d{self.sustain_duration:<2d} '
                f'R d{self.release_duration:<2d} '
                f'| mix {MIXER_FLAGS.get(self.mixer, hex(self.mixer))} '
                f'| oct {self.octave:+d}')

    def to_dict(self):
        d = {f: getattr(self, f) for f in self.FIELDS}
        d.update(mixer=self.mixer, octave_shift=self.octave,
                 raw=self.raw.hex())
        return d


class Event:
    """One decoded stream item."""

    def __init__(self, kind, offset, **kw):
        self.kind = kind
        self.offset = offset
        self.__dict__.update(kw)

    def __repr__(self):
        return f'<{self.kind}@{self.offset:#x}>'


class SNG:
    def __init__(self, data, name='<data>'):
        self.name = name
        self.data = data
        self.has_magic = data[:9] == MAGIC
        self.file_type = data[9] if self.has_magic else None
        self.warnings = []

        if not self.has_magic:
            self.warnings.append('no \\xCDMstudio\\xCD signature -- '
                                 'treating the whole file as an event stream')
            self.names_a = self.names_b = []
            self.envelopes = []
            self.chanmap = self.progmap = []
            self.title = ''
            self.trailer = b''
            self.ptrs = []
            self.stream_start = 0
        else:
            self.names_a = [cstr(data[OFF_NAMES_A + i * 10:OFF_NAMES_A + i * 10 + 10])
                            for i in range(N_VOICES)]
            self.envelopes = [Envelope(data[OFF_ENVELOPES + i * 8:
                                            OFF_ENVELOPES + i * 8 + 8])
                              for i in range(N_VOICES)]
            self.names_b = [cstr(data[OFF_NAMES_B + i * 10:OFF_NAMES_B + i * 10 + 10])
                            for i in range(N_VOICES)]
            self.chanmap = [b - 1 for b in data[OFF_CHANMAP:OFF_CHANMAP + N_VOICES]]
            self.progmap = [b - 1 for b in data[OFF_PROGMAP:OFF_PROGMAP + N_VOICES]]
            self.ptrs = list(struct.unpack('>5I', data[OFF_PTRS:OFF_PTRS + 20])) \
                if len(data) >= OFF_PTRS + 20 else []
            self.title = cstr(data[OFF_TITLE:OFF_TITLE + 32])
            self.trailer = data[OFF_TRAILER:OFF_TRAILER + 8]
            self.stream_start = OFF_STREAM

        # header-command defaults (mq_setp / globals)
        self.key = 1
        self.tempo = 120
        self.volume = None
        self.velocity = 0x7F
        self.psg_volume = 15
        self.header_cmds = []
        self.events = []
        self.notes = []
        self.total_units = 0
        self.stream_end = len(data)
        self.lyrics = ''
        self.lyrics_offset = None

        if self.file_type == TYPE_SONG or not self.has_magic:
            self._parse_header()
            self._parse_stream()
            self._parse_lyrics()

    # ------------------------------------------------------------ header
    def _parse_header(self):
        """mq_parh: walk header commands from mi_dbase to the first 0x00."""
        d = self.data
        p = self.stream_start
        if p < len(d) and d[p] == 0:
            p += 1
        while p < len(d) and d[p] != 0:
            b = d[p]
            if (b & 0x9F) < 0x20:               # stray note event: 3-byte skip
                self.header_cmds.append(('note-skip', p, d[p:p + 3].hex()))
                p += 3
                continue
            if b == 0x80:                       # key signature
                self.key = d[p + 2]
                self.header_cmds.append(('key', p, self.key))
                p += 3
            elif b == 0x81:                     # tempo, quarter notes/minute
                self.tempo = d[p + 1]
                self.header_cmds.append(('tempo', p, self.tempo))
                p += 2
            elif b == 0x83:                     # master volume (playback ignores)
                self.volume = d[p + 1]
                self.header_cmds.append(('volume', p, self.volume))
                p += 2
            elif b == 0x84:                     # default velocity -> PSG volume
                self.velocity = d[p + 2]
                v = self.velocity
                self.psg_volume = (5 if v < 0x17 else 7 if v < 0x27 else
                                   9 if v < 0x37 else 11 if v < 0x57 else
                                   13 if v < 0x67 else 15)
                self.header_cmds.append(('velocity', p, self.velocity))
                p += 3
            elif b == 0xC0:                     # program change (skipped here)
                self.header_cmds.append(('program', p, d[p + 1:p + 3].hex()))
                p += 3
            elif b == 0xFF:
                self.header_cmds.append(('end', p, None))
                break
            else:
                self.warnings.append(f'unknown header command {b:#04x} at {p:#x}')
                break
        self.header_end = p
        self.spb = 2400 // self.tempo if self.tempo else 20
        self.scale = build_scale_table(self.key)

    # ------------------------------------------------------------ stream
    def _parse_stream(self):
        """mq_pars + mq_rdur + mq_pshl/mq_popl, run to completion."""
        d = self.data
        p = self.header_end
        unit = 0                                # timeline, in 1/24 quarter notes
        stack = []                              # (return offset, remaining)
        active = {}                             # tied notes: (chan, note) -> index
        guard = 0
        while p < len(d):
            guard += 1
            if guard > 4_000_000:
                self.warnings.append('event walk aborted (runaway loop)')
                break
            if d[p] != 0:
                self.warnings.append(f'expected group separator at {p:#x}, '
                                     f'found {d[p]:#04x}')
                break
            p += 1
            if p >= len(d):
                break
            step, p = self._peek_step(p)
            first_note_seen = False
            done = False

            while p < len(d) and d[p] != 0:
                b = d[p]
                if not b & 0x80:                        # ---- note event
                    if p + 2 >= len(d):
                        done = True
                        break
                    b0, b1, b2 = d[p], d[p + 1], d[p + 2]
                    ev = self._note_event(p, b0, b1, b2, unit)
                    self.events.append(ev)
                    if ev.note_on:
                        self._emit_note(ev, active, unit)
                    first_note_seen = True
                    p += 3
                elif b == 0x82:                         # ---- bar marker
                    self.events.append(Event('bar', p, unit=unit))
                    p += 1
                    if not first_note_seen:
                        step, p = self._peek_step(p)
                elif b == 0x85:                         # ---- loop start
                    count = d[p + 1]
                    self.events.append(Event('loop_start', p, unit=unit,
                                             count=count))
                    if len(stack) < 20:
                        stack.append([p + 2, count - 1])
                    p += 2
                    step, p = self._peek_step(p)
                elif b == 0x86:                         # ---- loop end
                    self.events.append(Event('loop_end', p, unit=unit))
                    p += 1
                    if stack:
                        if stack[-1][1] == 0:
                            stack.pop()
                        else:
                            stack[-1][1] -= 1
                            p = stack[-1][0]
                    step, p = self._peek_step(p)
                elif b == 0xFF:                         # ---- end of song
                    self.events.append(Event('end', p, unit=unit))
                    done = True
                    break
                else:                                   # ---- 1-byte no-op
                    self.events.append(Event('unknown', p, unit=unit, byte=b))
                    p += 1
            if done:
                break
            unit += step
        self.total_units = unit
        self.stream_end = p
        # tied notes still open at the end get closed at the final position
        for key, idx in active.items():
            if self.notes[idx]['end'] is None:
                self.notes[idx]['end'] = unit
                self.warnings.append(
                    f'unterminated tie: voice {key[0]} note {note_name(key[1])}')

    def _parse_lyrics(self):
        """Anything after the 0xFF end marker is a lyric block.  The player
        never reads it; Music Studio displays it under the staff."""
        tail = self.data[self.stream_end + 1:]
        if tail and any(tail):
            self.lyrics_offset = self.stream_end + 1
            self.lyrics = tail.rstrip(b'\0').decode('latin1')

    def _peek_step(self, p):
        """mq_rdur: skip padding, then take the advance from the duration
        index of the note event about to be parsed.  Returns (step, p)."""
        d = self.data
        while p < len(d) and d[p] == 0:
            p += 1
        if p + 1 < len(d) and not d[p] & 0x80:
            return NDT[d[p + 1] & 0x1F], p
        return 0, p

    def _note_event(self, p, b0, b1, b2, unit):
        chan = b0 & 0x0F
        note_on = not b0 & 0x10
        note_off = bool(b0 & 0x20)
        tie = bool(b0 & 0x40)
        dur_idx = b1 & 0x1F
        accent = bool(b1 & 0x20)
        mode = b1 & 0xC0
        raw = b2 & 0x7F
        if mode == 0:
            note = self.scale[raw] if raw < len(self.scale) else raw
            acc = 'key'
        elif mode == 0x40:
            note, acc = raw, 'natural'
        elif mode == 0x80:
            note, acc = raw + 1, 'sharp'
        else:
            note, acc = raw - 1, 'flat'
        return Event('note', p, unit=unit, chan=chan, note_on=note_on,
                     note_off=note_off, tie=tie, dur_idx=dur_idx,
                     dur=NDT[dur_idx], accent=accent, accidental=acc,
                     raw_note=raw, note=note,
                     velocity=0x7F if accent else self.velocity,
                     bytes=bytes((b0, b1, b2)))

    def _emit_note(self, ev, active, unit):
        """mq_qnne: every event takes a queue slot, but bit 5 suppresses the
        Note-On and bit 6 suppresses the queued Note-Off.  A tied note is
        therefore  [tie] -> [tie|off]* -> [off]."""
        key = (ev.chan, ev.note)
        end = unit + max(ev.dur - 1.0 / self.spb, 0)
        if ev.note_off:                         # continuation of a tie
            idx = active.get(key)
            if idx is not None:
                if ev.tie:
                    self.notes[idx]['segments'] += 1
                else:
                    self.notes[idx]['end'] = end
                    del active[key]
            return
        rec = {'start': unit, 'end': None if ev.tie else end, 'chan': ev.chan,
               'note': ev.note, 'velocity': ev.velocity, 'dur_idx': ev.dur_idx,
               'tie': ev.tie, 'segments': 1, 'offset': ev.offset}
        self.notes.append(rec)
        if ev.tie:
            active[key] = len(self.notes) - 1

    # ------------------------------------------------------------ helpers
    def midi_channel(self, voice):
        """Physical MIDI channel for logical voice 1..15 (mq_dise MIDI path)."""
        return self.chanmap[voice - 1] & 0x0F if self.chanmap else 0

    def midi_transpose(self, voice):
        return -(3 - ((self.chanmap[voice - 1] >> 4) & 0xF)) * 12 if self.chanmap else 0

    def seconds(self, units):
        return units * self.spb / TICK_HZ

    # ------------------------------------------------------------ reports
    def report(self, show_events=False, out=sys.stdout):
        w = out.write
        w(f'{self.name}\n{"=" * len(self.name)}\n')
        w(f'  signature      {"yes" if self.has_magic else "NO"}'
          f'   type {self.file_type} '
          f'({ {1: "instrument set (.SND)", 2: "song (.SNG/.ORG)"}.get(self.file_type, "?") })\n')
        w(f'  size           {len(self.data)} bytes\n')
        if self.has_magic:
            w(f'  title          "{self.title}"\n')
            w(f'  pointers@0x1cc ' + ' '.join(f'{x:#08x}' for x in self.ptrs) + '\n')
            w(f'  trailer @0x200 {self.trailer.hex(" ")}\n')
        if self.file_type != TYPE_SOUND:
            w(f'  key signature  {self.key} ({KEY_NAMES[self.key & 15]} major, '
              f'mask {MSMK[self.key & 15]:#04x})\n')
            w(f'  tempo          {self.tempo} quarter notes/min '
              f'-> {self.spb} ticks per 1/24 note @ {TICK_HZ:.0f} Hz\n')
            w(f'  velocity       {self.velocity} (PSG volume {self.psg_volume})\n')
            if self.volume is not None:
                w(f'  volume         {self.volume}\n')
            w(f'  header cmds    ' +
              ', '.join(f'{k}={v}' for k, _o, v in self.header_cmds) + '\n')

        if self.has_magic:
            used = {n['chan'] for n in self.notes}
            w('\n  voice  instrument   (2nd list)   MIDI ch  transp  prog  envelope\n')
            for i in range(N_VOICES):
                v = i + 1
                w(('  *' if v in used else '   ') + f'{v:3d}  '
                  f'{self.names_a[i]:<12} {self.names_b[i]:<12} '
                  f'{self.midi_channel(v):7d} {self.midi_transpose(v):+6d} '
                  f'{self.progmap[i]:5d}  {self.envelopes[i].describe()}\n')

        if self.file_type == TYPE_SOUND:
            return

        w(f'\n  stream         {self.stream_start:#x}..{len(self.data):#x}, '
          f'header ends {self.header_end:#x}\n')
        kinds = {}
        for e in self.events:
            kinds[e.kind] = kinds.get(e.kind, 0) + 1
        w('  events         ' + ', '.join(f'{k}={n}' for k, n in sorted(kinds.items())) + '\n')
        w(f'  notes sounded  {len(self.notes)}\n')
        w(f'  length         {self.total_units} units = '
          f'{self.total_units / 24:.1f} quarter notes = '
          f'{self.seconds(self.total_units):.1f} s\n')
        w(f'  voices used    {sorted({n["chan"] for n in self.notes})} '
          f'(marked * above)\n')
        w(f'  stream walk    ended at {self.stream_end:#x} of {len(self.data):#x}\n')
        if self.notes:
            lo = min(n['note'] for n in self.notes)
            hi = max(n['note'] for n in self.notes)
            w(f'  range          {note_name(lo)}..{note_name(hi)} '
              f'(MIDI {lo}..{hi})\n')
        if self.lyrics:
            w(f'  lyrics         {len(self.lyrics)} bytes at '
              f'{self.lyrics_offset:#x}\n')
            for i in range(0, min(len(self.lyrics), 240), 72):
                w(f'                 |{self.lyrics[i:i + 72]}|\n')
            if len(self.lyrics) > 240:
                w('                 ...\n')
        for msg in self.warnings:
            w(f'  ! {msg}\n')

        if show_events:
            w('\n  offset   unit    bar  event\n')
            bar = 0
            for e in self.events:
                if e.kind == 'bar':
                    bar += 1
                    w(f'  {e.offset:#06x} {e.unit:7d} {bar:6d}  ---- bar ----\n')
                elif e.kind == 'note':
                    flags = ''.join((('T' if e.tie else '.'),
                                     ('O' if e.note_off else '.'),
                                     ('>' if e.accent else '.'),
                                     ('-' if not e.note_on else '.')))
                    w(f'  {e.offset:#06x} {e.unit:7d} {bar:6d}  '
                      f'{e.bytes.hex(" ")}  v{e.chan:<2d} {note_name(e.note):<5} '
                      f'{DUR_NAMES.get(e.dur, str(e.dur)):<6} {flags} {e.accidental}\n')
                elif e.kind == 'loop_start':
                    w(f'  {e.offset:#06x} {e.unit:7d} {bar:6d}  loop start x{e.count}\n')
                elif e.kind == 'loop_end':
                    w(f'  {e.offset:#06x} {e.unit:7d} {bar:6d}  loop end\n')
                elif e.kind == 'end':
                    w(f'  {e.offset:#06x} {e.unit:7d} {bar:6d}  END\n')
                else:
                    w(f'  {e.offset:#06x} {e.unit:7d} {bar:6d}  '
                      f'unknown byte {e.byte:#04x}\n')
        w('\n')

    def to_dict(self):
        return {
            'file': self.name, 'size': len(self.data),
            'signature': self.has_magic, 'type': self.file_type,
            'title': self.title, 'trailer': self.trailer.hex(),
            'key': self.key, 'key_name': KEY_NAMES[self.key & 15],
            'tempo': self.tempo, 'ticks_per_unit': self.spb,
            'velocity': self.velocity, 'psg_volume': self.psg_volume,
            'volume': self.volume,
            'instruments_a': self.names_a, 'instruments_b': self.names_b,
            'envelopes': [e.to_dict() for e in self.envelopes],
            'channel_map': self.chanmap, 'program_map': self.progmap,
            'header_commands': [{'cmd': k, 'offset': o, 'value': v}
                                for k, o, v in self.header_cmds],
            'length_units': self.total_units,
            'length_seconds': round(self.seconds(self.total_units), 3),
            'pointers': self.ptrs,
            'lyrics_offset': self.lyrics_offset, 'lyrics': self.lyrics,
            'notes': self.notes,
            'warnings': self.warnings,
        }

    # ------------------------------------------------------------ MIDI out
    def to_midi(self):
        """Format-1 MIDI file: one track per voice that actually plays, so the
        15 Music Studio voices survive being collapsed onto a handful of MIDI
        channels.  Division = 24*spb, so one MIDI tick is one 960 Hz sequencer
        tick and the timing is exact rather than rounded.  Channel, program and
        transposition follow the MIDI OUT path of mq_dise."""
        div = 24 * self.spb
        tracks = [self._midi_tempo_track()]
        for v in sorted({n['chan'] for n in self.notes}):
            trk = self._midi_voice_track(v)
            if trk:
                tracks.append(trk)
        head = b'MThd' + struct.pack('>IHHH', 6, 1, len(tracks), div)
        return head + b''.join(b'MTrk' + struct.pack('>I', len(t)) + t
                               for t in tracks)

    def _midi_tempo_track(self):
        trk = bytearray()
        usec = int(round(60_000_000 / self.tempo)) if self.tempo else 500000
        trk += b'\x00\xff\x51\x03' + usec.to_bytes(3, 'big')
        if self.title:
            trk += b'\x00\xff\x03' + _text(self.title)
        if self.lyrics:
            trk += b'\x00\xff\x01' + _text(self.lyrics)
        trk += b'\x00\xff\x2f\x00'
        return bytes(trk)

    def _midi_voice_track(self, voice):
        if not 1 <= voice <= N_VOICES:
            return None
        ch = self.midi_channel(voice)
        shift = self.midi_transpose(voice)
        events = []
        for n in self.notes:
            if n['chan'] != voice:
                continue
            note = n['note'] + shift
            if not 0 <= note <= 127:
                self.warnings.append(
                    f'voice {voice}: note {note} out of MIDI range, dropped')
                continue
            end = n['end'] if n['end'] is not None else n['start'] + 1
            on = int(round(n['start'] * self.spb))
            off = max(on + 1, int(round(end * self.spb)))
            events.append((on, 1, bytes((0x90 | ch, note, n['velocity'] & 0x7F))))
            events.append((off, 0, bytes((0x80 | ch, note, 0))))
        if not events:
            return None
        events.sort(key=lambda e: (e[0], e[1]))

        trk = bytearray()
        name = self.names_a[voice - 1] if self.names_a else f'voice {voice}'
        trk += b'\x00\xff\x03' + _text(f'{voice:02d} {name}')
        prog = self.progmap[voice - 1] if self.progmap else -1
        if 0 <= prog < 128:
            trk += b'\x00' + bytes((0xC0 | ch, prog))
        prev = 0
        for tick, _o, msg in events:
            trk += _varlen(tick - prev) + msg
            prev = tick
        trk += b'\x00\xff\x2f\x00'
        return bytes(trk)


def _text(s):
    b = s.encode('latin1')[:127]
    return bytes((len(b),)) + b


def _varlen(n):
    if n < 0:
        n = 0
    out = bytearray([n & 0x7F])
    n >>= 7
    while n:
        out.insert(0, (n & 0x7F) | 0x80)
        n >>= 7
    return bytes(out)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('files', nargs='+')
    ap.add_argument('--events', action='store_true',
                    help='list every stream event')
    ap.add_argument('--midi', metavar='OUT.MID',
                    help='write a standard MIDI file (single input file)')
    ap.add_argument('--json', action='store_true', help='machine-readable dump')
    args = ap.parse_args()

    songs = [SNG(open(f, 'rb').read(), f) for f in args.files]

    if args.json:
        json.dump([s.to_dict() for s in songs], sys.stdout, indent=2)
        sys.stdout.write('\n')
        return

    if args.midi:
        if len(songs) != 1:
            ap.error('--midi takes exactly one input file')
        open(args.midi, 'wb').write(songs[0].to_midi())
        print(f'wrote {args.midi} ({len(songs[0].notes)} notes, '
              f'{songs[0].tempo} bpm)')
        return

    for s in songs:
        s.report(show_events=args.events)


if __name__ == '__main__':
    main()
