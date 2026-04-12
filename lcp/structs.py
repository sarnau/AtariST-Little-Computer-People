"""
Data structures for Little Computer People (Atari ST).
Translated from Ghidra struct definitions on LCP.PRG.

Original platform: Motorola 68000, big-endian, 16-bit int (short).
All structs are mapped to the original binary layout for save-file
compatibility with the DATA/HYBER file.
"""

import struct
from dataclasses import dataclass, field
from typing import List

from .enums import (
    ACTION_ID, SICKNESS_LEVEL, HAPPINESS_LEVEL, NEED_LEVEL,
    FACING_DIR, CLOTHING_COLOR_ID, SKIN_COLOR_ID, ENV_PHASE,
)


# ---------------------------------------------------------------------------
# LCP — 128-byte character save file struct
# addr: lcp global at 0x501B8, loaded from DATA/HYBER by lcp_load()
# All numeric fields are 16-bit big-endian words (2 bytes each).
# Field offsets verified from 68000 assembly disassembly of:
#   lcp_create_random (0x169D8), lcp_load (0x15AC8),
#   game_simulate_one_second (0x233DA), action_write_letter (0x23F66)
#
# door_states_and_flags bit layout:
#   Bit 0: front_door (open/closed)
#   Bit 1: study_door
#   Bit 2: closet_door
#   Bit 3: kitchen_cabinet
#   Bit 4: dresser
#   Bit 5: toilet_door
#   Bit 6: filing_cabinet
#   Bits 7-8: dog_bowl_status
# ---------------------------------------------------------------------------
@dataclass
class LCP:
    """
    128-byte persistent character state.  Mirrors the binary HYBER save file.
    Field order, sizes, and offsets match the original Alcyon C struct layout
    as verified against Ghidra disassembly.
    """
    # -- Appearance (offset 0x00–0x03) -----------------------------------------
    clothing_color: int         = 0    # 0x00: CLOTHING_COLOR_ID (0–15)
    skin_color: int             = 0    # 0x02: SKIN_COLOR_ID (0–7)

    # -- Daily Schedule (offset 0x04–0x0B) -------------------------------------
    bedtime_hour: int           = 22   # 0x04: 22–24 (wraps: 24 becomes 0)
    wake_hour: int              = 7    # 0x06: bedtime+6 (wraps past 23), 6–8
    lunch_hour: int             = 12   # 0x08: 11–13
    dinner_hour: int            = 18   # 0x0A: 17–19

    # -- Personality (offset 0x0C–0x0F) ----------------------------------------
    personality_type: int       = 0    # 0x0C: PERSONALITY_TYPE (0–3)
    activity_level: int         = 4    # 0x0E: 0–7; selects action table

    # -- Reserved (offset 0x10–0x27, 24 bytes) ---------------------------------
    _reserved_10: bytes         = field(default=b'\x00' * 24, repr=False)

    # -- Happiness (offset 0x28–0x33) ------------------------------------------
    happiness: int              = 0    # 0x28: 0=happy, 1=content, 2=sad
    happiness_initial_countdown: int = 12  # 0x2A: 6–24 hours
    happiness_duration_happy: int = 12 # 0x2C: 6–24 hours
    happiness_duration_content: int = 8 # 0x2E: 6–12 hours
    happiness_duration_active: int = 0 # 0x30: countdown; reloaded from duration
    happiness_direction: int    = 1    # 0x32: +1=toward sad, -1=toward happy

    # -- Sickness (offset 0x34–0x39) -------------------------------------------
    sickness_level: int         = 0    # 0x34: SICKNESS_LEVEL (0=healthy..4=critical)
    sickness_countdown: int     = 0    # 0x36: minutes to next level change
    sickness_direction: int     = 1    # 0x38: +1=worsening, -1=improving, 0=stable

    # -- Sleep (offset 0x3A) ---------------------------------------------------
    is_sleeping: int            = 0    # 0x3A: 0=awake, 1=sleeping

    # -- Initiative (offset 0x3C) ----------------------------------------------
    initiative_threshold: int   = 50   # 0x3C: 20–80; lower = more proactive

    # -- Thirst (offset 0x3E–0x43) ---------------------------------------------
    thirst_level: int           = 0    # 0x3E: 0–3 (3 triggers sickness)
    thirst_timer_max: int       = 60   # 0x40: 45–75 game-minutes
    thirst_timer: int           = 0    # 0x42: countdown to thirst_level++

    # -- Hunger (offset 0x44–0x49) ---------------------------------------------
    hunger_level: int           = 0    # 0x44: 0–3 (3 triggers sickness)
    hunger_timer_max: int       = 90   # 0x46: 75–120 game-minutes
    hunger_timer: int           = 0    # 0x48: countdown to hunger_level++

    # -- Bathroom (offset 0x4A–0x4F) -------------------------------------------
    bathroom_need: int          = 0    # 0x4A: 0=no, 1=yes
    bathroom_timer_max: int     = 30   # 0x4C: 20–40 game-minutes
    bathroom_timer: int         = 0    # 0x4E: countdown; at 0 → need=1, timer=9999

    # -- Reserved (offset 0x50–0x51) -------------------------------------------
    _reserved_50: int           = 0    # 0x50: no code references

    # -- Items / State (offset 0x52–0x59) --------------------------------------
    food_supply: int            = 4    # 0x52: food count
    record_playing: int         = 0    # 0x54: 0=off, 1=on
    tv_on: int                  = 0    # 0x56: 0=off, 1=on
    door_states_and_flags: int  = 0    # 0x58: bit-packed door/furniture state

    # -- Character ID (offset 0x5A–0x5D) ---------------------------------------
    character_sprite_id: int    = 2    # 0x5A: 2–6 (PE2..PE6.LCP)
    water_level: int            = 7    # 0x5C: water level

    # -- Names (offset 0x5E–0x7F) ----------------------------------------------
    owner_name: bytes           = field(default=b'\x00' * 24)   # 0x5E: player name (24 bytes)
    character_name: bytes       = field(default=b'\x00' * 10)   # 0x76: LCP name (10 bytes)

    # -- Struct packing --------------------------------------------------------
    # Layout: 8 shorts + 24 bytes + 27 shorts + 24 bytes + 10 bytes = 128 bytes
    #   8 words (0x00–0x0F) + 24 pad (0x10–0x27) + 27 words (0x28–0x5D) + 24 name + 10 name
    _PACK_FMT = '>8h24s27h24s10s'
    _PACK_SIZE = 128

    @classmethod
    def from_bytes(cls, data: bytes) -> 'LCP':
        """Parse a 128-byte HYBER save file into an LCP instance."""
        assert len(data) >= cls._PACK_SIZE, f"HYBER too short: {len(data)}"
        f = struct.unpack(cls._PACK_FMT, data[:cls._PACK_SIZE])
        return cls(
            clothing_color          = f[0],
            skin_color              = f[1],
            bedtime_hour            = f[2],
            wake_hour               = f[3],
            lunch_hour              = f[4],
            dinner_hour             = f[5],
            personality_type        = f[6],
            activity_level          = f[7],
            _reserved_10            = f[8],
            happiness               = f[9],
            happiness_initial_countdown = f[10],
            happiness_duration_happy = f[11],
            happiness_duration_content = f[12],
            happiness_duration_active = f[13],
            happiness_direction     = f[14],
            sickness_level          = f[15],
            sickness_countdown      = f[16],
            sickness_direction      = f[17],
            is_sleeping             = f[18],
            initiative_threshold    = f[19],
            thirst_level            = f[20],
            thirst_timer_max        = f[21],
            thirst_timer            = f[22],
            hunger_level            = f[23],
            hunger_timer_max        = f[24],
            hunger_timer            = f[25],
            bathroom_need           = f[26],
            bathroom_timer_max      = f[27],
            bathroom_timer          = f[28],
            _reserved_50            = f[29],
            food_supply             = f[30],
            record_playing          = f[31],
            tv_on                   = f[32],
            door_states_and_flags   = f[33],
            character_sprite_id     = f[34],
            water_level             = f[35],
            owner_name              = f[36],
            character_name          = f[37],
        )

    def to_bytes(self) -> bytes:
        """Serialise back to 128-byte HYBER format."""
        # Ensure we have indices 0..37 matching _PACK_FMT
        # Indices 0-7: 8 shorts, 8: 24-byte pad, 9-35: 27 shorts, 36: 24-byte name, 37: 10-byte name
        return struct.pack(
            self._PACK_FMT,
            self.clothing_color,
            self.skin_color,
            self.bedtime_hour,
            self.wake_hour,
            self.lunch_hour,
            self.dinner_hour,
            self.personality_type,
            self.activity_level,
            self._reserved_10[:24].ljust(24, b'\x00'),
            self.happiness,
            self.happiness_initial_countdown,
            self.happiness_duration_happy,
            self.happiness_duration_content,
            self.happiness_duration_active,
            self.happiness_direction,
            self.sickness_level,
            self.sickness_countdown,
            self.sickness_direction,
            self.is_sleeping,
            self.initiative_threshold,
            self.thirst_level,
            self.thirst_timer_max,
            self.thirst_timer,
            self.hunger_level,
            self.hunger_timer_max,
            self.hunger_timer,
            self.bathroom_need,
            self.bathroom_timer_max,
            self.bathroom_timer,
            self._reserved_50,
            self.food_supply,
            self.record_playing,
            self.tv_on,
            self.door_states_and_flags,
            self.character_sprite_id,
            self.water_level,
            self.owner_name[:24].ljust(24, b'\x00'),
            self.character_name[:10].ljust(10, b'\x00'),
        )

    @property
    def name_str(self) -> str:
        """Return character name as a stripped ASCII string."""
        return self.character_name.rstrip(b'\x00').decode('ascii', errors='replace')

    @property
    def owner_name_str(self) -> str:
        """Return owner/player name as a stripped ASCII string."""
        return self.owner_name.rstrip(b'\x00').decode('ascii', errors='replace')

    # -- Door state bit helpers ------------------------------------------------
    @property
    def front_door_open(self) -> bool:
        return bool(self.door_states_and_flags & 0x01)

    @property
    def study_door_open(self) -> bool:
        return bool(self.door_states_and_flags & 0x02)

    @property
    def closet_door_open(self) -> bool:
        return bool(self.door_states_and_flags & 0x04)

    @property
    def kitchen_cabinet_open(self) -> bool:
        return bool(self.door_states_and_flags & 0x08)

    @property
    def dresser_open(self) -> bool:
        return bool(self.door_states_and_flags & 0x10)

    @property
    def toilet_door_open(self) -> bool:
        return bool(self.door_states_and_flags & 0x20)

    @property
    def filing_cabinet_open(self) -> bool:
        return bool(self.door_states_and_flags & 0x40)

    @property
    def dog_bowl_status(self) -> int:
        return (self.door_states_and_flags >> 7) & 0x03


# ---------------------------------------------------------------------------
# PSG_ENVELOPE — 14-byte software ADSR envelope for YM2149 PSG channel
# addr: psg_envelope[3], psg_process_envelopes()
# One instance per PSG channel (3 channels total)
# ---------------------------------------------------------------------------
@dataclass
class PSG_ENVELOPE:
    """
    Software ADSR envelope processor for one YM2149 channel.
    Processed at 50 Hz by psg_process_envelopes() via MFP Timer A.
    """
    phase: int              = 0   # ENV_PHASE
    attack_start_vol: int   = 0   # initial volume at note-on
    attack_duration: int    = 0   # ticks to reach attack target
    attack_target_vol: int  = 0   # peak volume after attack
    decay_duration: int     = 0   # ticks from peak to sustain level
    decay_target_vol: int   = 0   # volume at end of decay
    sustain_duration: int   = 0   # ticks to hold sustain
    sustain_target_vol: int = 0   # volume during sustain
    release_duration: int   = 0   # ticks to fade to silence
    max_volume: int         = 15  # clamp ceiling (0–15)
    phase_timer: int        = 0   # countdown within current phase
    current_volume: int     = 0   # current output volume (0–15)
    ramp_direction: int     = 0   # +1 or -1 for Bresenham volume ramp

    _PACK_FMT = '>13b'   # 13 signed bytes; 14th byte is padding in binary
    _PACK_SIZE = 14

    @classmethod
    def from_bytes(cls, data: bytes) -> 'PSG_ENVELOPE':
        fields = struct.unpack(cls._PACK_FMT, data[:13])
        return cls(
            phase              = fields[0],
            attack_start_vol   = fields[1],
            attack_duration    = fields[2],
            attack_target_vol  = fields[3],
            decay_duration     = fields[4],
            decay_target_vol   = fields[5],
            sustain_duration   = fields[6],
            sustain_target_vol = fields[7],
            release_duration   = fields[8],
            max_volume         = fields[9],
            phase_timer        = fields[10],
            current_volume     = fields[11],
            ramp_direction     = fields[12],
        )


# ---------------------------------------------------------------------------
# MFDB — Memory Form Definition Block (Atari ST GEM VDI raster structure)
# Used by sprite_draw() for vro_cpyfm masked blit calls
# addr: sprite_mfdb_image[8], sprite_mfdb_mask[8]
# In Python this is a plain data container; no actual GEM calls are made
# ---------------------------------------------------------------------------
@dataclass
class MFDB:
    """
    Atari ST GEM VDI Memory Form Definition Block.
    Describes a bitmap for vro_cpyfm raster operations.
    Python equivalent — used for sprite compositing in render.py.
    """
    data: bytes       = b''   # raw bitplane pixel data
    width: int        = 0     # width in pixels
    height: int       = 0     # height in pixels
    wdwidth: int      = 0     # width in 16-bit words (ceil(width/16))
    standard: int     = 0     # 0 = device-specific, 1 = standard form
    bitplanes: int    = 4     # always 4 for Atari ST low resolution


# ---------------------------------------------------------------------------
# FILE_IMG_DATA — header for sprite/object data files (SPRITES, OBJECTS)
# addr: spritedata_load(), loadSpritesOrObjects() in readFiles.py
# Each sprite entry in the file begins with this 4-byte header
# ---------------------------------------------------------------------------
@dataclass
class FILE_IMG_DATA:
    """
    Header record for entries in the SPRITES and OBJECTS binary files.
    Immediately followed by height × ceil(width/16) × 4 bitplanes of pixel data.
    """
    height: int = 0   # image height in scanlines (big-endian short)
    width: int  = 0   # image width in pixels (big-endian short)

    _PACK_FMT  = '>HH'
    _PACK_SIZE = 4

    @classmethod
    def from_bytes(cls, data: bytes) -> 'FILE_IMG_DATA':
        h, w = struct.unpack(cls._PACK_FMT, data[:cls._PACK_SIZE])
        return cls(height=h, width=w)

    @property
    def byte_size(self) -> int:
        """Total pixel data size in bytes following this header."""
        import math
        words_per_row = math.ceil(self.width / 16)
        return self.height * words_per_row * 4 * 2  # 4 bitplanes × 2 bytes/word
