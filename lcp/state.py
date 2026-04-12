"""
GameState — central game state container for Little Computer People (Atari ST).

All ~200 global variables from LCP.PRG are consolidated here as attributes
of a single GameState instance.  This replaces the original C global namespace
(BSS segment 0x02C6C0–0x05A2F9).

One GameState instance is created at startup and passed by reference to every
subsystem.  This makes the simulation deterministic and testable without any
hardware or OS dependencies.
"""

from dataclasses import dataclass, field
from typing import List, Optional

from .enums import (
    ACTION_ID, PLAYER_STATE, HOUSE_POS, SPRITE_LAYER, SPRITE_ID,
    FACING_DIR, SICKNESS_LEVEL, HAPPINESS_LEVEL, NEED_LEVEL,
    DOG_BOWL_STATUS, HEAD_ANIM_MODE, MIDI_SEQ_PHASE, ENV_PHASE,
)
from .structs import LCP, PSG_ENVELOPE, MFDB


@dataclass
class GameState:
    """
    All runtime game state, grouped by subsystem.
    Attribute names match Ghidra symbol names exactly where possible.
    """

    # -----------------------------------------------------------------------
    # LCP character persistent struct (loaded from DATA/HYBER)
    # addr: lcp (global pointer to LCP struct)
    # -----------------------------------------------------------------------
    lcp: LCP = field(default_factory=LCP)

    # -----------------------------------------------------------------------
    # Core loop flags
    # addr: lcp_loaded, copyprot_check_return, game_speed_counter
    # -----------------------------------------------------------------------
    lcp_loaded: int            = 0    # 1 once save file is loaded
    copyprot_check_return: int = 1    # always 1 in Python (copy protection bypassed)
    game_speed_counter: int    = 0    # frame counter incremented each tick

    # -----------------------------------------------------------------------
    # In-game clock and calendar
    # addr: time_hours, time_minutes, game_seconds_counter,
    #       date_day, date_month, date_year
    # -----------------------------------------------------------------------
    time_hours: int         = 8    # 0–23
    time_minutes: int       = 0    # 0–59
    game_seconds_counter: int = 0  # 0–59 within current game-minute
    date_day: int           = 0    # 0–30
    date_month: int         = 0    # 0–11
    date_year: int          = 0

    # -----------------------------------------------------------------------
    # LCP character position and movement
    # addr: lcp_x, lcp_y, walk_target_x, walk_target_y,
    #       walk_waypoint_x, walk_waypoint_y, lcp_facing_direction,
    #       lcp_on_stairs_flag, lcp_state
    # -----------------------------------------------------------------------
    lcp_x: int                  = 0
    lcp_y: int                  = 0
    walk_target_x: int          = 0
    walk_target_y: int          = 0
    walk_waypoint_x: int        = 0
    walk_waypoint_y: int        = 0
    lcp_facing_direction: int   = FACING_DIR.FACING_RIGHT
    lcp_on_stairs_flag: int     = 0    # 1 while climbing/descending
    lcp_state: int              = PLAYER_STATE.STATE_STAND_IDLE

    # -----------------------------------------------------------------------
    # LCP carry / object state
    # addr: lcp_carrying_object_flag, lcp_carried_object
    # -----------------------------------------------------------------------
    lcp_carrying_object_flag: int  = 0
    lcp_carried_object: int        = 0   # SPRITE_ID of carried item
    lcp_sprites_hidden: int        = 0   # 1 = off-screen debug mode
    debug_hide_lcp_offscreen: int  = 0   # 1 = force sprites off-screen (Y=300)

    # -----------------------------------------------------------------------
    # Head animation
    # addr: head_sprite_frame, head_anim_current, head_anim_target,
    #       head_anim_mode, head_anim_delay_countdown,
    #       head_sprite_mirror_flag
    # -----------------------------------------------------------------------
    head_sprite_frame: int         = 0
    head_anim_current: int         = 0
    head_anim_target: int          = 0
    head_anim_mode: int            = HEAD_ANIM_MODE.HEAD_ANIM_NORMAL
    head_anim_delay_countdown: int = 0
    head_sprite_mirror_flag: int   = 0   # 0=normal, 1=mirror head horizontally

    # -----------------------------------------------------------------------
    # Palette state (Python equivalent of main_colorpalette[])
    # addr: main_colorpalette[6] — skin colour entry
    # -----------------------------------------------------------------------
    palette_skin_color: int        = 0x0742  # ST_PEACH default

    # -----------------------------------------------------------------------
    # Animation frame counter (incremented each render tick, ~8 Hz)
    # addr: animation_tick_counter
    # -----------------------------------------------------------------------
    animation_tick_counter: int = 0

    # -----------------------------------------------------------------------
    # AI / action system
    # addr: trigger_action, last_action, action_interruptible_flag,
    #       triggered_event_list[10], in_execute_event_routine_flag,
    #       intro_sequence_active
    # -----------------------------------------------------------------------
    trigger_action: int              = ACTION_ID.ACTION_NONE
    last_action: int                 = ACTION_ID.ACTION_NONE
    action_interruptible_flag: int   = 0
    triggered_event_list: List[int]  = field(default_factory=lambda: [ACTION_ID.ACTION_NONE] * 10)
    in_execute_event_routine_flag: int = 0
    intro_sequence_active: int       = 0

    # Command queue (player-typed commands)
    # addr: _action_queue[10], _action_priority_queue[10], _action_list_size
    action_queue: List[int]          = field(default_factory=lambda: [ACTION_ID.ACTION_NONE] * 10)
    action_priority_queue: List[int] = field(default_factory=lambda: [0] * 10)
    action_list_size: int            = 0

    # -----------------------------------------------------------------------
    # Daily reset flags (cleared at midnight by game_simulate_one_second)
    # addr: lunch_meal_triggered_today, dinner_meal_triggered_today,
    #       morning_wakeup_triggered_today, bedtime_triggered_today
    # -----------------------------------------------------------------------
    lunch_meal_triggered_today: int   = 0
    dinner_meal_triggered_today: int  = 0
    morning_wakeup_triggered_today: int = 0
    bedtime_triggered_today: int      = 0

    # -----------------------------------------------------------------------
    # Keyboard / event triggers
    # addr: ctrl_a_alarm_pressed_flag, phone_answered_flag,
    #       phone_call_active_flag, ctrl_b_book_flag, ctrl_d_dog_food_flag,
    #       ctrl_f_food_flag, ctrl_r_record_flag, ctrl_w_water_flag,
    #       ctrl_p_pet_flag, ctrl_c_phone_flag
    # -----------------------------------------------------------------------
    ctrl_a_alarm_pressed_flag: int  = 0
    phone_answered_flag: int        = 0
    phone_call_active_flag: int     = 0
    ctrl_b_book_flag: int           = 0
    ctrl_d_dog_food_flag: int       = 0
    ctrl_f_food_flag: int           = 0
    ctrl_r_record_flag: int         = 0
    ctrl_w_water_flag: int          = 0
    ctrl_p_pet_flag: int            = 0
    ctrl_c_phone_flag: int          = 0

    # -----------------------------------------------------------------------
    # Water supply
    # addr: lcp_water_level
    # -----------------------------------------------------------------------
    lcp_water_level: int = 100   # arbitrary max

    # -----------------------------------------------------------------------
    # Dog state
    # addr: dog_x, dog_y, dog_target_x, dog_target_y,
    #       dog_waypoint_x, dog_waypoint_y, dog_facing_direction,
    #       dog_on_stairs_flag, dog_walk_anim_cycle, dog_idle_countdown,
    #       dog_bowl_status, dog_is_eating, dog_petting_frame
    # -----------------------------------------------------------------------
    dog_x: int                  = 8
    dog_y: int                  = 190
    dog_target_x: int           = 0
    dog_target_y: int           = 0
    dog_waypoint_x: int         = 0
    dog_waypoint_y: int         = 0
    dog_facing_direction: int   = FACING_DIR.FACING_RIGHT
    dog_on_stairs_flag: int     = 0
    dog_walk_anim_cycle: int    = 0    # 0–7 walk frame index
    dog_idle_countdown: int     = 0    # ticks until next wander target
    dog_bowl_status: int        = DOG_BOWL_STATUS.BOWL_EMPTY
    dog_is_eating: int          = 0
    dog_eating_anim_cycle: int  = 0
    dog_petting_frame: int      = 0    # 0 = not being petted
    dog_near_food_bowl: int     = 0    # addr: dog_near_food_bowl
    dog_eating_active: int      = 0    # addr: dog_eating_active
    dog_eating_countdown: int   = 0    # addr: dog_eating_countdown
    dog_last_target_index: int  = -1   # addr: dog_last_target_index
    dog_initialized: int        = 0    # addr: dog_initialized (0=ready, 1=not yet on screen)
    dog_sprite_id: int          = 33   # addr: dog_sprite_id (current sprite being displayed) — default SPRITE_DOG_LAY_DOWN

    # -----------------------------------------------------------------------
    # Sprite pipeline — 3-level: definition / pending / active
    # 60 logical sprite slots, 8 hardware rendering slots
    # addr: sprite_def_image[60], sprite_def_mask[60], etc.
    # -----------------------------------------------------------------------
    # Definition level (loaded at startup from SPRITES file)
    sprite_def_image:  List[Optional[bytes]] = field(default_factory=lambda: [None] * 60)
    sprite_def_mask:   List[Optional[bytes]] = field(default_factory=lambda: [None] * 60)
    sprite_def_width:  List[int]             = field(default_factory=lambda: [0] * 60)
    sprite_def_height: List[int]             = field(default_factory=lambda: [0] * 60)

    # Layer flags for each logical sprite
    # addr: sprite_layer_flags[60]
    sprite_layer_flags: List[int] = field(default_factory=lambda: [SPRITE_LAYER.SPRITE_HIDDEN] * 60)

    # Slot assignment map: logical_sprite_id → hardware_slot (0–7), -1 if unassigned
    # addr: sprite_slot_map[60]
    sprite_slot_map: List[int] = field(default_factory=lambda: [-1] * 60)

    # Pending buffer (staged by game logic, committed each frame)
    # addr: sprite_pending_image[8], sprite_pending_x[8], etc.
    sprite_pending_image:  List[Optional[bytes]] = field(default_factory=lambda: [None] * 8)
    sprite_pending_mask:   List[Optional[bytes]] = field(default_factory=lambda: [None] * 8)
    sprite_pending_x:      List[int]             = field(default_factory=lambda: [0] * 8)
    sprite_pending_y:      List[int]             = field(default_factory=lambda: [0] * 8)
    sprite_pending_width:  List[int]             = field(default_factory=lambda: [0] * 8)
    sprite_pending_height: List[int]             = field(default_factory=lambda: [0] * 8)
    sprite_pending_flag:   List[int]             = field(default_factory=lambda: [0] * 8)

    # Active buffer (consumed by renderer each frame)
    # addr: sprite_active_image[8], sprite_active_x[8], etc.
    sprite_active_image:  List[Optional[bytes]] = field(default_factory=lambda: [None] * 8)
    sprite_active_mask:   List[Optional[bytes]] = field(default_factory=lambda: [None] * 8)
    sprite_active_x:      List[int]             = field(default_factory=lambda: [0] * 8)
    sprite_active_y:      List[int]             = field(default_factory=lambda: [0] * 8)
    sprite_active_width:  List[int]             = field(default_factory=lambda: [0] * 8)
    sprite_active_height: List[int]             = field(default_factory=lambda: [0] * 8)

    # LCP body/head composite buffers (4-word-wide expanded sprites)
    # addr: lcp_body_flip_buffer, lcp_head_flip_buffer
    lcp_body_flip_buffer: bytes = b'\x00' * (4 * 2 * 21)   # 4 words × 2 bytes × 21 lines
    lcp_head_flip_buffer: bytes = b'\x00' * (4 * 2 * 21)

    # -----------------------------------------------------------------------
    # Screen / rendering
    # addr: text_scroll_timer, offscreen buffer pointers, display buffer index
    # -----------------------------------------------------------------------
    text_scroll_timer: int   = 0
    display_buffer_idx: int  = 0   # double-buffer index (0 or 1)

    # -----------------------------------------------------------------------
    # Sound system
    # addr: midi_output_enabled, psg_output_enabled, soundeffect_pending,
    #       psg_envelope[3], midi_seq_phase, midi_seq_*
    # -----------------------------------------------------------------------
    midi_output_enabled: int  = 0
    psg_output_enabled: int   = 1
    soundeffect_pending: int  = -1   # -1 = none, else SOUND_EFFECT_ID

    # Sound effect runtime state
    # addr: soundeffect_active_flag, soundeffect_playing_flag, soundeffect_playing_id,
    #       soundeffect_remaining_ticks, soundeffect_duration, soundeffect_current
    soundeffect_active_flag: bool  = False    # True = effect queued to play this frame
    soundeffect_playing_flag: bool = False    # True = effect currently audible
    soundeffect_playing_id: int    = -1       # currently playing SOUND_EFFECT_ID
    soundeffect_remaining_ticks: int = 0      # ticks until effect ends
    soundeffect_duration: int      = 0        # total duration of queued effect

    psg_envelope: List[PSG_ENVELOPE] = field(
        default_factory=lambda: [PSG_ENVELOPE(), PSG_ENVELOPE(), PSG_ENVELOPE()]
    )

    # PSG Bresenham ramp state (one per channel)
    # addr: psg_channel_ramp_delta[], psg_channel_ramp_accum[]
    psg_ramp_delta:   List[int] = field(default_factory=lambda: [0, 0, 0])
    psg_ramp_accum:   List[int] = field(default_factory=lambda: [0, 0, 0])
    psg_channel_volume: List[int] = field(default_factory=lambda: [0, 0, 0])
    psg_notes_active: bool = False

    midi_seq_phase: int       = MIDI_SEQ_PHASE.SEQ_WAIT_NOTE_EXPIRE
    midi_seq_position: int    = 0    # byte offset into current song data
    midi_seq_countdown: int   = 0    # ticks until next event
    midi_song_data: bytes     = b''  # loaded .SNG or .ORG file bytes
    midi_song_active: int     = 0    # 1 = song playing

    # MIDI sequencer tick state (from midi_seq_tick_handler)
    # addr: midi_tick_counter, midi_tick_prescaler, midi_tick_divider, midi_reentrant_lock
    midi_tick_counter: int    = 0
    midi_tick_prescaler: int  = 4
    midi_tick_divider: int    = 4
    midi_reentrant_lock: bool = False

    footstep_trigger_flag: int = 0

    # -----------------------------------------------------------------------
    # Environmental objects and animations
    # addr: fire_animation_frame, alarm_active, phone_ringing,
    #       tv_on, record_player_on, doorbell_count
    # -----------------------------------------------------------------------
    fire_animation_frame: int  = 0
    fire_active: int           = 0
    fire_active_flag: int      = 0     # alias used in game_tick
    fire_extinguish_flag: int  = 0
    fire_duration_countdown: int = 0
    alarm_active: int          = 0
    alarm_sound_started: int   = 0
    phone_ringing: int         = 0
    phone_call_active_flag: int = 0    # set by simulation.py when call comes in
    phone_ring_countdown: int  = 0
    phone_hangup_flag: int     = 0
    phone_answered_flag: int   = 0
    tv_on: int                 = 0
    lcp_tv_on: int             = 0    # alias used in game_tick
    record_player_on: int      = 0
    lcp_record_playing: int    = 0    # alias used in game_tick
    doorbell_count: int        = 0
    clock_animation_frame: int = 0

    # Petting animation state
    # addr: petting_dog_active, petting_anim_frame, petting_last_sprite_slot
    petting_dog_active: int    = 0
    petting_anim_frame: int    = 0
    petting_last_sprite_slot: int = 0

    # Screen scroll state
    # addr: screen_scroll_down_count
    screen_scroll_down_count: int = 0

    # Sub-frame animation counter (incremented every game_tick frame)
    # addr: sub_animation_frame_counter
    sub_animation_frame_counter: int = 0

    # Dog visibility flag (toggled by action_play_a_game)
    # addr: dog_visible
    dog_visible: int = 1

    # Dog food bowl delta (set by game_tick per frame)
    # addr: dog_food_bowl_change  (+1 add food, -1 remove, 0 no change)
    dog_food_bowl_change: int = 0

    # Keyboard input mode flag (set during certain mini-game screens)
    # addr: game_input_mode_flag
    game_input_mode_flag: int = 0

    # disable_key_input_flag: set during mini-games / cinematics
    # addr: disable_key_input_flag
    disable_key_input_flag: int = 0

    # Keyboard command input
    # addr: command_input_buffer_pos
    command_input_buffer_pos: int = 0

    # -----------------------------------------------------------------------
    # Copy protection (always passes in Python reimplementation)
    # addr: copyprot_check_return
    # -----------------------------------------------------------------------
    # copyprot_check_return already declared above = 1

    # -----------------------------------------------------------------------
    # NLP / keyboard input
    # addr: keyboard_input_buffer, entered_word_bits[10]
    # -----------------------------------------------------------------------
    keyboard_input_buffer: str    = ''
    entered_word_bits: List[int]  = field(default_factory=lambda: [0] * 10)

    # -----------------------------------------------------------------------
    # Delivery flag — set when Ctrl+F/B/R/D deliveries arrive
    # addr: delivery_is_for_dog, food_package_available, etc.
    # -----------------------------------------------------------------------
    delivery_is_for_dog: int      = 0
    food_package_available: int   = 0
    book_available: int           = 0
    record_delivery_available: int = 0

    # -----------------------------------------------------------------------
    # Mini-game state
    # addr: current_game_type, card_deck[], etc.
    # -----------------------------------------------------------------------
    current_game_type: int  = -1   # -1 = none
    card_deck: List[int]    = field(default_factory=lambda: list(range(52)))

    def reset_daily_flags(self) -> None:
        """Reset flags that are cleared at midnight. Called by game_simulate_one_second."""
        self.lunch_meal_triggered_today   = 0
        self.dinner_meal_triggered_today  = 0
        self.morning_wakeup_triggered_today = 0
        self.bedtime_triggered_today      = 0
