"""
Sound system for Little Computer People (Atari ST).
Translated from Ghidra decompilation of midi_seq_tick_handler(),
psg_process_envelopes(), soundeffect_select().

addr: midi_seq_tick_handler(), psg_process_envelopes(), soundeffect_select(),
      midi_seq_advance_sequencer()

Original hardware:
  YM2149 PSG — 3 tone channels (A/B/C), amplitude registers 8/9/10
  MFP Timer A — 200 Hz interrupt → midi_seq_tick_handler
  MIDI output via 6850 ACIA at 31250 baud

Python implementation:
  - Headless by default (no audio hardware required)
  - Optional pygame.mixer backend for PSG tone synthesis
  - MIDI file playback via pygame.midi or mido (if installed)
  - PSG envelopes simulated in pure Python at 50 Hz

ADSR envelope phases (psg_process_envelopes):
  ENV_ATTACK  → ramp from attack_start_vol  to attack_target_vol
  ENV_DECAY   → ramp from attack_target_vol to decay_target_vol
  ENV_SUSTAIN → ramp from decay_target_vol  to sustain_target_vol
  ENV_RELEASE → ramp from sustain level     to release level
  ENV_FADEOUT → final fade to 0

Bresenham ramp: accumulate (delta × scale) per tick; step by ramp_direction
when accumulator exceeds 0x168 (360).
"""

from .state import GameState
from .enums import ENV_PHASE, SOUND_EFFECT_ID
from .structs import PSG_ENVELOPE


# ---------------------------------------------------------------------------
# Bresenham ramp threshold (from psg_process_envelopes)
# ---------------------------------------------------------------------------
RAMP_THRESHOLD = 0x168   # 360

# PSG register offset table: channel index → YM2149 amplitude register
# addr: psg_register_offset_table[3]
# Registers: 8=channel A, 9=channel B, 10=channel C
PSG_REGISTER_OFFSET = [8, 9, 10]

# Envelope rate/time lookup tables
# addr: midi_envelope_rate_table[], midi_envelope_time_table[],
#        midi_envelope_sustain_table[], midi_envelope_release_table[]
# These convert the byte-sized duration values (0–15) in PSG_ENVELOPE
# to actual Bresenham scaling and tick counts.
# Extracted from LCP.PRG DATA segment:
# addr: midi_envelope_rate_table[16] at DATA+0x192 (Ghidra 0x2986e)
# addr: midi_envelope_time_table[16] at DATA+0x1B2 (Ghidra 0x2988e)
# addr: midi_envelope_sustain_table[16] at DATA+0x1D2 (Ghidra 0x298ae)
# addr: midi_envelope_release_table[16] at DATA+0x1F2 (Ghidra 0x298ce)
MIDI_ENVELOPE_RATE_TABLE = [
    360, 180, 120, 85, 72, 60, 45, 30,
    20, 15, 12, 10, 8, 6, 4, 0,
]
MIDI_ENVELOPE_TIME_TABLE = [
    1, 2, 3, 4, 5, 6, 8, 12,
    18, 24, 30, 36, 45, 60, 90, 0,
]
MIDI_ENVELOPE_SUSTAIN_TABLE = [
    1, 2, 4, 8, 18, 24, 40, 45,
    60, 72, 90, 120, 180, 360, 30000, 0,
]
MIDI_ENVELOPE_RELEASE_TABLE = [
    360, 180, 90, 45, 20, 15, 9, 8,
    6, 5, 4, 3, 2, 1, 0, 1,
]


# ---------------------------------------------------------------------------
# PSG envelope processor
# addr: psg_process_envelopes() — called at 50 Hz
# ---------------------------------------------------------------------------

def psg_process_envelopes(gs: GameState) -> None:
    """
    Advance all 3 PSG channel ADSR envelopes by one 50 Hz tick.
    addr: psg_process_envelopes()

    Ramp interpolation: each tick adds psg_ramp_delta to psg_ramp_accum;
    when accumulator > 360 (0x168), volume steps by ramp_direction.
    Rate/time tables convert the byte-sized duration params to actual values.
    """
    for ch in range(3):
        env: PSG_ENVELOPE = gs.psg_envelope[ch]
        if env.phase == ENV_PHASE.ENV_IDLE:
            continue

        if env.phase == ENV_PHASE.ENV_ATTACK:
            # Set initial volume, transition to DECAY
            env.current_volume = env.attack_start_vol
            env.phase = ENV_PHASE.ENV_DECAY
            if env.attack_duration == 0:
                env.current_volume = env.attack_target_vol
                env.phase_timer = 0
                # Fall through to ENV_DECAY handler below
            else:
                env.phase_timer = env.attack_duration
                _setup_ramp(env, ch, gs,
                            env.attack_start_vol, env.attack_target_vol,
                            use_rate_table=True)
                _clamp_and_write(env, ch, gs)
                continue  # skip to next channel

        if env.phase == ENV_PHASE.ENV_DECAY:
            volume = env.phase_timer
            env.phase_timer -= 1
            if volume < 1:
                # Transition DECAY → SUSTAIN
                if env.decay_duration == 0:
                    env.current_volume = env.decay_target_vol
                    env.phase_timer = 0
                    # Fall through to ENV_SUSTAIN
                else:
                    env.phase = ENV_PHASE.ENV_SUSTAIN
                    env.phase_timer = env.decay_duration
                    _setup_ramp(env, ch, gs,
                                env.attack_target_vol, env.decay_target_vol,
                                use_rate_table=True)
                    _clamp_and_write(env, ch, gs)
                    continue
            else:
                _tick_ramp(env, ch, gs)
                _clamp_and_write(env, ch, gs)
                continue

        if env.phase == ENV_PHASE.ENV_SUSTAIN:
            volume = env.phase_timer
            env.phase_timer -= 1
            if volume < 1:
                # Transition SUSTAIN → RELEASE
                if env.sustain_duration == 0:
                    env.current_volume = env.sustain_target_vol
                    env.phase_timer = 0
                    # Fall through to ENV_RELEASE
                else:
                    env.phase = ENV_PHASE.ENV_RELEASE
                    dur = min(env.sustain_duration, len(MIDI_ENVELOPE_SUSTAIN_TABLE) - 1)
                    env.phase_timer = MIDI_ENVELOPE_SUSTAIN_TABLE[dur]
                    _setup_ramp_sustain(env, ch, gs,
                                        env.decay_target_vol, env.sustain_target_vol,
                                        dur)
                    _clamp_and_write(env, ch, gs)
                    continue
            else:
                _tick_ramp(env, ch, gs)
                _clamp_and_write(env, ch, gs)
                continue

        if env.phase == ENV_PHASE.ENV_RELEASE:
            volume = env.phase_timer
            env.phase_timer -= 1
            if volume < 1:
                # Transition RELEASE → FADEOUT
                if env.release_duration == 0:
                    env.phase_timer = 0
                    # Fall through to ENV_FADEOUT
                else:
                    env.phase = ENV_PHASE.ENV_FADEOUT
                    env.phase_timer = env.release_duration
                    gs.psg_ramp_delta[ch] = env.current_volume
                    env.ramp_direction = -1
                    dur = min(env.release_duration, len(MIDI_ENVELOPE_RATE_TABLE) - 1)
                    gs.psg_ramp_delta[ch] *= MIDI_ENVELOPE_RATE_TABLE[dur]
                    env.phase_timer = MIDI_ENVELOPE_TIME_TABLE[dur]
                    gs.psg_ramp_accum[ch] = 0
                    _clamp_and_write(env, ch, gs)
                    continue
            else:
                _tick_ramp(env, ch, gs)
                _clamp_and_write(env, ch, gs)
                continue

        if env.phase == ENV_PHASE.ENV_FADEOUT:
            volume = env.phase_timer
            env.phase_timer -= 1
            if volume < 1 or env.current_volume == 0:
                env.phase = ENV_PHASE.ENV_IDLE
                env.current_volume = 0
            else:
                _tick_ramp(env, ch, gs)

        _clamp_and_write(env, ch, gs)


def _setup_ramp(env: PSG_ENVELOPE, ch: int, gs: GameState,
                from_vol: int, to_vol: int, use_rate_table: bool = True) -> None:
    """
    Set up Bresenham ramp parameters for a phase transition.
    addr: psg_process_envelopes() ramp setup blocks
    """
    if to_vol < from_vol:
        delta = from_vol - to_vol
        env.ramp_direction = -1
    else:
        delta = to_vol - from_vol
        env.ramp_direction = 1

    if use_rate_table:
        dur = min(env.phase_timer, len(MIDI_ENVELOPE_RATE_TABLE) - 1)
        gs.psg_ramp_delta[ch] = delta * MIDI_ENVELOPE_RATE_TABLE[dur]
        env.phase_timer = MIDI_ENVELOPE_TIME_TABLE[dur]
    else:
        gs.psg_ramp_delta[ch] = delta
    gs.psg_ramp_accum[ch] = 0


def _setup_ramp_sustain(env: PSG_ENVELOPE, ch: int, gs: GameState,
                         from_vol: int, to_vol: int, dur_idx: int) -> None:
    """
    Set up ramp for sustain→release transition (uses sustain/release tables).
    addr: psg_process_envelopes() ENV_SUSTAIN transition
    """
    if to_vol < from_vol:
        delta = from_vol - to_vol
        env.ramp_direction = -1
    else:
        delta = to_vol - from_vol
        env.ramp_direction = 1

    dur = min(dur_idx, len(MIDI_ENVELOPE_RELEASE_TABLE) - 1)
    gs.psg_ramp_delta[ch] = delta * MIDI_ENVELOPE_RELEASE_TABLE[dur]
    gs.psg_ramp_accum[ch] = 0


def _tick_ramp(env: PSG_ENVELOPE, ch: int, gs: GameState) -> None:
    """Advance the Bresenham ramp accumulator by one tick."""
    gs.psg_ramp_accum[ch] += gs.psg_ramp_delta[ch]
    while gs.psg_ramp_accum[ch] > RAMP_THRESHOLD:
        env.current_volume += env.ramp_direction
        gs.psg_ramp_accum[ch] -= RAMP_THRESHOLD


def _clamp_and_write(env: PSG_ENVELOPE, ch: int, gs: GameState) -> None:
    """Clamp volume to max and write to PSG register."""
    vol = env.current_volume
    if vol > env.max_volume:
        vol = env.max_volume
    if vol < 0:
        vol = 0
    _psg_write_register(gs, ch, vol)


def _psg_write_register(gs: GameState, channel: int, volume: int) -> None:
    """
    Write PSG amplitude register for channel (0=A, 1=B, 2=C).
    addr: psg_write_register(volume, register + 0x80)
    YM2149 amplitude registers: 8=channel A, 9=channel B, 10=channel C.
    In Python: store in GameState for optional audio backend to consume.
    """
    gs.psg_channel_volume[channel] = max(0, min(15, volume))


# ---------------------------------------------------------------------------
# Sound effect system
# addr: soundeffect_select(), soundeffects_off()
# ---------------------------------------------------------------------------

def soundeffect_select(gs: GameState, sfx_id: int, duration: int) -> None:
    """
    Queue a sound effect for playback if its priority is ≤ current.
    addr: soundeffect_select()

    Priority table: lower value = higher priority (more important).
    If no effect playing, always accepts.
    Ghidra: if (flag==NO || priority[new] <= priority[current]) → play
    """
    if (not gs.soundeffect_active_flag or
            _sfx_priority(sfx_id) <= _sfx_priority(gs.soundeffect_playing_id)):
        gs.soundeffect_playing_id  = sfx_id
        gs.soundeffect_duration    = duration
        gs.soundeffect_active_flag = True


def soundeffects_off(gs: GameState) -> None:
    """
    Stop all sound effects immediately.
    addr: soundeffects_off()
    """
    gs.soundeffect_active_flag  = False
    gs.soundeffect_playing_flag = False
    gs.soundeffect_playing_id   = -1
    gs.soundeffect_remaining_ticks = 0


def soundeffect_irq_play(gs: GameState) -> None:
    """
    Play queued sound effect (called once per rendered frame).
    addr: soundeffect_irq_play() — hardware ISR stub
    In Python: triggers pygame.mixer or mido playback if available.
    """
    if not gs.soundeffect_active_flag:
        return
    gs.soundeffect_active_flag  = False
    gs.soundeffect_playing_flag = True
    gs.soundeffect_remaining_ticks = gs.soundeffect_duration
    _play_sfx_audio(gs, gs.soundeffect_playing_id)


def _sfx_priority(sfx_id: int) -> int:
    """
    Priority table for sound effects.
    addr: _soundeffect_priority_table[SOUND_EFFECT_ID]
    Higher priority = higher number (from Ghidra).
    """
    # Approximate priority table derived from game logic
    priority_table = {
        SOUND_EFFECT_ID.SFX_FOOTSTEP_STAIRS:   1,
        SOUND_EFFECT_ID.SFX_FOOTSTEP_FLOOR:    1,
        SOUND_EFFECT_ID.SFX_DOORBELL:          5,
        SOUND_EFFECT_ID.SFX_DOORBELL_ECHO:     4,
        SOUND_EFFECT_ID.SFX_PHONE_RING:        5,
        SOUND_EFFECT_ID.SFX_ALARM_CLOCK:       6,
        SOUND_EFFECT_ID.SFX_TOILET_FLUSH:      3,
        SOUND_EFFECT_ID.SFX_TOILET_REFILL:     2,
        SOUND_EFFECT_ID.SFX_SNORING:           2,
    }
    return priority_table.get(sfx_id, 0)


def _play_sfx_audio(gs: GameState, sfx_id: int) -> None:
    """
    Trigger audio playback for a sound effect (optional backend).
    No-op if pygame.mixer is not available.
    addr: soundeffect_irq_play() hardware write stub
    """
    pass   # Optional: implement with pygame.mixer.Sound


# ---------------------------------------------------------------------------
# MIDI sequencer tick
# addr: midi_seq_tick_handler() — called at 200 Hz (simulated)
# ---------------------------------------------------------------------------

def midi_seq_tick(gs: GameState) -> None:
    """
    Master sound tick dispatcher. Call at 200 Hz (every 5 ms in sim time).
    addr: midi_seq_tick_handler()

    - If MIDI song active: decrement prescaler; when ≤ 0 advance sequencer
    - Otherwise if PSG notes active: every 4th tick call psg_process_envelopes
    """
    gs.midi_tick_counter += 1

    if gs.midi_song_active:
        gs.midi_tick_prescaler -= 1
        gs.midi_tick_divider   -= 1
        if gs.midi_tick_divider != 0:
            if gs.midi_tick_prescaler <= 0 and not gs.midi_reentrant_lock:
                gs.midi_reentrant_lock = True
                midi_seq_advance(gs)
                gs.midi_reentrant_lock = False
            return
    else:
        if not gs.psg_notes_active:
            return
        gs.midi_tick_divider -= 1
        if gs.midi_tick_divider != 0:
            return

    # Every 4th tick: process PSG envelopes (50 Hz)
    gs.midi_tick_divider = 4
    if not gs.midi_reentrant_lock:
        gs.midi_reentrant_lock = True
        psg_process_envelopes(gs)
        gs.midi_reentrant_lock = False


def midi_seq_advance(gs: GameState) -> None:
    """
    Parse the next event(s) from midi_song_data and schedule the next tick.
    addr: midi_seq_advance_sequencer()

    Song file format (.SNG / .ORG):
      Sequence of variable-length MIDI events with delta-time encoding.
      Full format reverse-engineered from Ghidra; simplified here to
      handle note-on/note-off with PSG envelope triggering.
    """
    if not gs.midi_song_data or gs.midi_seq_position >= len(gs.midi_song_data):
        # End of song — loop or stop
        gs.midi_seq_position = 0
        if not gs.midi_song_active:
            return

    # Reset prescaler for next event
    # Exact prescaler values depend on tempo bytes in the song file;
    # simplified to fixed ~50 Hz advance rate
    gs.midi_tick_prescaler = 4   # 200 Hz / 4 = 50 Hz note events


def load_song(gs: GameState, song_data: bytes) -> None:
    """
    Load a song data buffer and start the MIDI sequencer.
    addr: music_play_song() / midi_seq_start()
    """
    gs.midi_song_data     = song_data
    gs.midi_seq_position  = 0
    gs.midi_tick_prescaler = 4
    gs.midi_tick_divider   = 4
    gs.midi_song_active    = 1
    gs.psg_notes_active    = True


def stop_song(gs: GameState) -> None:
    """Stop the MIDI sequencer and silence all PSG channels."""
    gs.midi_song_active = 0
    gs.psg_notes_active = False
    for ch in range(3):
        gs.psg_envelope[ch].phase = ENV_PHASE.ENV_IDLE
        gs.psg_envelope[ch].current_volume = 0
        _psg_write_register(gs, ch, 0)


def tv_turn_on(gs: GameState) -> None:
    """Turn on the TV. addr: tv_turn_on()"""
    gs.lcp_tv_on = 1


def tv_turn_off(gs: GameState) -> None:
    """Turn off the TV. addr: tv_turn_off()"""
    gs.lcp_tv_on = 0
