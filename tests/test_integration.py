"""
Integration tests for Little Computer People (Atari ST) Python reimplementation.

Covers:
  1. End-to-end simulation: running game_simulate_one_second + screen_render_8hz_headless
     in a tight loop for a full game-day (and multi-day stress test).
  2. Save/Load verification: HYBER round-trip, field-value checks, load_all_assets.
  3. Copy protection bypass verification: copyprot_check_return behaviour.
"""

import os
import sys
import random
import tempfile
import copy
from pathlib import Path
from unittest.mock import patch, MagicMock

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from lcp.state import GameState
from lcp.structs import LCP
from lcp.enums import (
    ACTION_ID, PLAYER_STATE, FACING_DIR, HOUSE_POS,
    SPRITE_ID, DOG_BOWL_STATUS, SICKNESS_LEVEL, HAPPINESS_LEVEL,
    HEAD_ANIM_MODE, SOUND_EFFECT_ID,
)
from lcp.constants import house_get_position_xy
from lcp.render import screen_render_8hz_headless
from lcp.simulation import game_simulate_one_second
from lcp.sprites import sprite_update_body, sprite_lcp_head_animate, sprite_lcp_head_update
from lcp.assets import load_hyber, save_hyber, load_all_assets


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

DATA_DIR = Path(__file__).parent.parent / "DATA"
HYBER_PATH = DATA_DIR / "HYBER"

# 1 game-second = 8 frames (animation_tick_counter increments each frame,
# game_simulate_one_second acts when counter & 7 == 0).
FRAMES_PER_SECOND = 8
SECONDS_PER_MINUTE = 60
MINUTES_PER_HOUR = 60
HOURS_PER_DAY = 24

FRAMES_PER_MINUTE = FRAMES_PER_SECOND * SECONDS_PER_MINUTE          # 480
FRAMES_PER_HOUR = FRAMES_PER_MINUTE * MINUTES_PER_HOUR              # 28800
FRAMES_PER_DAY = FRAMES_PER_HOUR * HOURS_PER_DAY                    # 691200


def _make_gs(**overrides) -> GameState:
    """Create a GameState suitable for headless simulation."""
    gs = GameState()
    gs.lcp = LCP()
    gs.copyprot_check_return = 1
    gs.lcp_loaded = 0          # don't auto-position at study door
    gs.game_speed_counter = 5
    gs.lcp_x = 160
    gs.lcp_y = 190
    gs.lcp_state = PLAYER_STATE.STATE_STAND_IDLE
    gs.lcp_facing_direction = FACING_DIR.FACING_RIGHT
    gs.lcp.happiness = 0
    gs.lcp.sickness_level = 0
    gs.lcp.hunger_level = 0
    gs.lcp.thirst_level = 0
    gs.lcp.food_supply = 4
    gs.lcp.thirst_timer = 60
    gs.lcp.hunger_timer = 90
    gs.lcp.bathroom_timer = 30
    gs.lcp.happiness_duration_active = 12
    gs.dog_x = 8
    gs.dog_y = 190
    gs.dog_sprite_id = SPRITE_ID.SPRITE_DOG_LAY_DOWN
    gs.time_hours = 8
    gs.time_minutes = 0
    gs.game_seconds_counter = 0
    gs.date_day = 0
    gs.date_month = 0
    gs.date_year = 0
    for k, v in overrides.items():
        setattr(gs, k, v)
    return gs


def _run_frames(gs: GameState, n: int) -> None:
    """Run n headless frames (render + simulate)."""
    for _ in range(n):
        screen_render_8hz_headless(gs)
        game_simulate_one_second(gs)


def _run_frames_with_sprites(gs: GameState, n: int) -> None:
    """Run n headless frames with sprite updates."""
    for _ in range(n):
        screen_render_8hz_headless(gs)
        game_simulate_one_second(gs)
        sprite_update_body(gs)
        sprite_lcp_head_animate(gs)
        sprite_lcp_head_update(gs)


# ===================================================================
# 1. End-to-end integration tests
# ===================================================================

class TestEndToEndSimulation:
    """Run the game simulation loop for extended periods."""

    def test_one_game_day_no_crash(self):
        """Run 691,200 frames (1 full game-day) without any exception."""
        gs = _make_gs()
        random.seed(42)
        _run_frames(gs, FRAMES_PER_DAY)
        # If we got here, no crash occurred.

    def test_clock_advances_one_day(self):
        """Start at 8:00, run 24 hours of frames, verify clock is back at 8:00."""
        gs = _make_gs()
        random.seed(42)
        # We need to run from 8:00 through midnight (16h) then 8 more hours = 24h total.
        _run_frames(gs, FRAMES_PER_DAY)
        assert gs.time_hours == 8, f"Expected hour 8, got {gs.time_hours}"
        assert gs.time_minutes == 0, f"Expected minute 0, got {gs.time_minutes}"

    def test_calendar_advances(self):
        """Start at day 0, verify day increments after 24 game-hours."""
        gs = _make_gs()
        random.seed(42)
        initial_day = gs.date_day
        # Run from 8:00 AM to midnight = 16 hours
        frames_to_midnight = FRAMES_PER_HOUR * 16
        _run_frames(gs, frames_to_midnight)
        assert gs.date_day == initial_day + 1, (
            f"Expected day {initial_day + 1}, got {gs.date_day}"
        )

    def test_needs_increase_over_time(self):
        """Verify hunger/thirst levels increase after enough game-minutes."""
        gs = _make_gs()
        random.seed(42)
        gs.lcp.thirst_timer = 1     # will tick to 0 on first minute
        gs.lcp.hunger_timer = 1     # will tick to 0 on first minute
        gs.lcp.thirst_level = 0
        gs.lcp.hunger_level = 0
        # Run 2 game-minutes worth of frames (enough for timers to fire)
        _run_frames(gs, FRAMES_PER_MINUTE * 2)
        assert gs.lcp.thirst_level >= 1, (
            f"Expected thirst >= 1, got {gs.lcp.thirst_level}"
        )
        assert gs.lcp.hunger_level >= 1, (
            f"Expected hunger >= 1, got {gs.lcp.hunger_level}"
        )

    def test_daily_flags_reset_at_midnight(self):
        """Set daily flags, run past midnight, verify cleared."""
        gs = _make_gs()
        random.seed(42)
        gs.time_hours = 23
        gs.time_minutes = 58
        gs.game_seconds_counter = 0
        gs.lunch_meal_triggered_today = 1
        gs.dinner_meal_triggered_today = 1
        gs.morning_wakeup_triggered_today = 1
        gs.bedtime_triggered_today = 1
        # Run 3 minutes (enough to cross midnight at 23:58 -> 00:01)
        _run_frames(gs, FRAMES_PER_MINUTE * 3)
        assert gs.lunch_meal_triggered_today == 0
        assert gs.dinner_meal_triggered_today == 0
        assert gs.morning_wakeup_triggered_today == 0
        assert gs.bedtime_triggered_today == 0

    def test_sickness_progression(self):
        """Set sickness_level=1 with worsening direction, verify countdown decrements."""
        gs = _make_gs()
        random.seed(42)
        gs.lcp.sickness_level = 1   # SICKNESS_LEVEL_1
        gs.lcp.sickness_countdown = 5
        gs.lcp.sickness_direction = -1  # improving
        initial_countdown = gs.lcp.sickness_countdown
        # Run 3 game-minutes; countdown should decrement each minute
        _run_frames(gs, FRAMES_PER_MINUTE * 3)
        assert gs.lcp.sickness_countdown < initial_countdown, (
            f"Expected countdown < {initial_countdown}, got {gs.lcp.sickness_countdown}"
        )

    def test_happiness_cycle(self):
        """Verify happiness changes over several game-hours."""
        gs = _make_gs()
        random.seed(42)
        gs.lcp.happiness = HAPPINESS_LEVEL.MOOD_HAPPY
        gs.lcp.happiness_direction = 1  # toward sad
        gs.lcp.happiness_duration_active = 1  # will fire on first hour boundary
        gs.lcp.happiness_initial_countdown = 2
        gs.lcp.happiness_duration_happy = 2
        gs.lcp.happiness_duration_content = 2
        # Run enough hours for happiness to change at least once
        _run_frames(gs, FRAMES_PER_HOUR * 4)
        # happiness should have moved from 0 (happy) to at least 1 (content) by now
        assert gs.lcp.happiness != HAPPINESS_LEVEL.MOOD_HAPPY or gs.lcp.happiness_direction != 1, (
            "Expected happiness to have cycled at least once"
        )

    def test_dog_ai_runs_without_crash(self):
        """Dog AI runs every frame via screen_render_8hz_headless without crash."""
        gs = _make_gs()
        random.seed(42)
        gs.dog_x = 50
        gs.dog_y = 190
        gs.dog_target_x = 0
        gs.dog_target_y = 0
        # Run 1000 frames -- dog AI runs each frame inside screen_render_8hz_headless
        _run_frames(gs, 1000)

    def test_sprite_updates_every_frame(self):
        """Verify sprite_update_body/head don't crash over many frames."""
        gs = _make_gs()
        random.seed(42)
        _run_frames_with_sprites(gs, 2000)

    def test_phone_call_can_trigger(self):
        """Run frames between 8-22h, verify phone_call_active_flag eventually set."""
        gs = _make_gs()
        random.seed(12345)  # deterministic seed
        gs.time_hours = 10
        gs.time_minutes = 0
        gs.game_seconds_counter = 0
        gs.phone_answered_flag = 0
        gs.intro_sequence_active = 0
        # Run several game-hours; 2% chance per minute means very likely in 600 minutes
        # We keep hours in range 8-21 so the condition stays valid
        triggered = False
        for _ in range(FRAMES_PER_MINUTE * 600):
            screen_render_8hz_headless(gs)
            game_simulate_one_second(gs)
            if gs.phone_call_active_flag:
                triggered = True
                break
        assert triggered, "Expected phone_call_active_flag to be set at least once"

    def test_full_simulation_stress(self):
        """Run for 2 game-days (1,382,400 frames) -- verify no crash, calendar advanced by 2."""
        gs = _make_gs()
        random.seed(42)
        initial_day = gs.date_day
        _run_frames(gs, FRAMES_PER_DAY * 2)
        # Calendar should have advanced by 2 days from the start hour (8:00).
        # After 2 full days of frames starting at 8:00, we should be back at 8:00
        # and the date should have advanced by 2.
        assert gs.time_hours == 8
        assert gs.date_day == initial_day + 2


# ===================================================================
# 2. Save/Load verification (HYBER round-trip)
# ===================================================================

class TestSaveLoad:
    """Test the HYBER save file round-trip and field values."""

    def test_load_hyber_file(self):
        """Load DATA/HYBER, verify it returns an LCP with character name 'Norton'."""
        lcp = load_hyber(HYBER_PATH)
        assert isinstance(lcp, LCP)
        assert "Norton" in lcp.name_str

    def test_hyber_round_trip(self):
        """Load -> to_bytes -> from_bytes, verify all fields match."""
        original = load_hyber(HYBER_PATH)
        raw = original.to_bytes()
        restored = LCP.from_bytes(raw)
        assert restored.clothing_color == original.clothing_color
        assert restored.skin_color == original.skin_color
        assert restored.wake_hour == original.wake_hour
        assert restored.lunch_hour == original.lunch_hour
        assert restored.dinner_hour == original.dinner_hour
        assert restored.personality_type == original.personality_type
        assert restored.activity_level == original.activity_level
        assert restored.happiness == original.happiness
        assert restored.sickness_level == original.sickness_level
        assert restored.sickness_direction == original.sickness_direction
        assert restored.character_sprite_id == original.character_sprite_id
        assert restored.name_str == original.name_str
        assert restored.owner_name_str == original.owner_name_str
        assert restored.food_supply == original.food_supply
        assert restored.thirst_level == original.thirst_level
        assert restored.hunger_level == original.hunger_level

    def test_hyber_byte_identity(self):
        """Load raw bytes, parse to LCP, serialize back, assert bytes are identical."""
        raw_original = HYBER_PATH.read_bytes()
        lcp = LCP.from_bytes(raw_original)
        raw_reserialized = lcp.to_bytes()
        assert raw_reserialized == raw_original, (
            "Reserialized bytes differ from original HYBER file"
        )

    def test_lcp_default_to_bytes_round_trip(self):
        """Create default LCP(), to_bytes, from_bytes, verify all fields."""
        original = LCP()
        raw = original.to_bytes()
        assert len(raw) == 128
        restored = LCP.from_bytes(raw)
        assert restored.clothing_color == original.clothing_color
        assert restored.skin_color == original.skin_color
        assert restored.bedtime_hour == original.bedtime_hour
        assert restored.wake_hour == original.wake_hour
        assert restored.happiness == original.happiness
        assert restored.sickness_level == original.sickness_level
        assert restored.food_supply == original.food_supply
        assert restored.character_name == original.character_name
        assert restored.owner_name == original.owner_name

    def test_hyber_field_values(self):
        """Load HYBER and verify specific known values from the hex dump."""
        lcp = load_hyber(HYBER_PATH)
        assert lcp.clothing_color == 2
        assert lcp.skin_color == 7
        assert lcp.wake_hour == 6
        assert lcp.lunch_hour == 13
        assert lcp.dinner_hour == 18
        assert lcp.personality_type == 3
        assert lcp.activity_level == 6
        assert lcp.happiness == 0      # happy
        assert lcp.sickness_level == 0  # healthy
        assert lcp.sickness_direction == -1
        assert lcp.character_sprite_id == 6
        assert "Norton" in lcp.name_str
        assert "REBECCA" in lcp.owner_name_str

    def test_door_state_bits(self):
        """Load HYBER, verify door_states_and_flags bit helpers work correctly."""
        lcp = load_hyber(HYBER_PATH)
        # door_states_and_flags = 2114 = 0x0842
        # bit 0: front_door    = 0x0842 & 0x01 = 0 -> closed
        # bit 1: study_door    = 0x0842 & 0x02 = 2 -> open
        # bit 6: filing_cabinet = 0x0842 & 0x40 = 0x40 -> open
        assert lcp.front_door_open == False
        assert lcp.study_door_open == True
        assert lcp.filing_cabinet_open == True
        # Verify the properties are bool-like and don't crash
        _ = lcp.closet_door_open
        _ = lcp.kitchen_cabinet_open
        _ = lcp.dresser_open
        _ = lcp.toilet_door_open
        _ = lcp.dog_bowl_status

    def test_save_and_reload(self):
        """Load HYBER, modify a field, save to temp file, reload, verify changed."""
        original = load_hyber(HYBER_PATH)
        original.food_supply = 99
        with tempfile.NamedTemporaryFile(suffix=".hyber", delete=False) as f:
            tmp_path = Path(f.name)
        try:
            save_hyber(original, tmp_path)
            reloaded = load_hyber(tmp_path)
            assert reloaded.food_supply == 99
            assert reloaded.name_str == original.name_str
        finally:
            tmp_path.unlink(missing_ok=True)

    def test_load_all_assets_loads_hyber(self):
        """Call load_all_assets, verify gs.lcp_loaded == 1 and character name is 'Norton'."""
        gs = GameState()
        load_all_assets(gs, data_dir=DATA_DIR)
        assert gs.lcp_loaded == 1
        assert "Norton" in gs.lcp.name_str


# ===================================================================
# 3. Copy protection bypass verification
# ===================================================================

class TestCopyProtection:
    """Verify copy protection is always bypassed in the Python reimplementation."""

    def test_copyprot_default_passes(self):
        """GameState default has copyprot_check_return = 1."""
        gs = GameState()
        assert gs.copyprot_check_return == 1

    def test_copyprot_blocks_when_zero(self):
        """If copyprot_check_return = 0, endless_game_loop would enter sleep-forever loop."""
        from lcp.main import endless_game_loop
        gs = _make_gs()
        gs.copyprot_check_return = 0
        gs.lcp_loaded = 0
        # The endless_game_loop enters `while True: action_sleep(gs, -1)` when
        # copyprot_check_return == 0. We patch action_sleep to raise after first call
        # to verify the branch is entered.
        call_count = [0]

        def fake_sleep(gs_arg, val):
            call_count[0] += 1
            if call_count[0] >= 1:
                raise StopIteration("sleep-forever branch entered")

        with patch("lcp.actions.action_sleep", side_effect=fake_sleep):
            with pytest.raises(StopIteration, match="sleep-forever"):
                endless_game_loop(gs)

    def test_game_loop_entry_with_copyprot(self):
        """With copyprot_check_return = 1, the game loop proceeds past the check."""
        from lcp.main import endless_game_loop
        gs = _make_gs()
        gs.copyprot_check_return = 1
        gs.lcp_loaded = 0
        # Patch game_tick_and_animate to break out after first call
        call_count = [0]

        def fake_tick(gs_arg, counter):
            call_count[0] += 1
            if call_count[0] >= 1:
                raise StopIteration("game loop running")

        with patch("lcp.main.game_tick_and_animate", side_effect=fake_tick):
            with pytest.raises(StopIteration, match="game loop running"):
                endless_game_loop(gs)
        assert call_count[0] >= 1

    def test_lcp_positioned_at_study_door_when_loaded(self):
        """Set lcp_loaded = 1, mock the main loop to break, verify LCP at study door."""
        from lcp.main import endless_game_loop
        gs = _make_gs()
        gs.copyprot_check_return = 1
        gs.lcp_loaded = 1

        def fake_tick(gs_arg, counter):
            raise StopIteration("break")

        with patch("lcp.main.game_tick_and_animate", side_effect=fake_tick):
            with pytest.raises(StopIteration):
                endless_game_loop(gs)

        # Study door position
        expected_x, expected_y = house_get_position_xy(HOUSE_POS.POS_TOP_STUDY_DOOR)
        assert gs.lcp_x == expected_x - 10
        assert gs.lcp_y == expected_y - 3


# ===================================================================
# 4. Timing model verification
# ===================================================================

class TestTimingModel:
    """
    Verify the timing model matches the original Atari ST hardware.

    Original timing chain:
      200 Hz MFP Timer A → 25 ticks → 8 Hz screen_render_8hz
      8 render frames → 1 game-second (animation_tick_counter & 7 == 0)
      60 game-seconds → 1 game-minute (time_minutes += 1)
      60 game-minutes → 1 game-hour (time_hours += 1)
      24 game-hours → 1 game-day (date_day += 1)

    At 8 Hz:
      1 game-second = 8 frames × 125ms = 1.000 real second
      1 game-minute = 480 frames = 60 real seconds
      1 game-hour = 28,800 frames = 3,600 real seconds = 1 real hour
      1 game-day = 691,200 frames = 86,400 real seconds = 1 real day

    The game runs in real time (1:1 game-time to wall-clock time).
    """

    def test_simulation_gate_every_8_frames(self):
        """game_simulate_one_second only acts when animation_tick_counter & 7 == 0."""
        gs = _make_gs()
        gs.game_seconds_counter = 0
        gs.animation_tick_counter = 0

        # Manually step through 8 frames
        for i in range(8):
            game_simulate_one_second(gs)
            gs.animation_tick_counter += 1

        # After 8 frames: exactly 1 game-second elapsed
        assert gs.game_seconds_counter == 1

    def test_simulation_gate_skips_non_aligned_frames(self):
        """Frames where counter & 7 != 0 should NOT advance simulation."""
        gs = _make_gs()
        gs.game_seconds_counter = 0
        gs.animation_tick_counter = 1  # not aligned

        # Try 7 non-aligned frames
        for i in range(7):
            game_simulate_one_second(gs)
            gs.animation_tick_counter += 1

        # No game-seconds should have elapsed
        assert gs.game_seconds_counter == 0

    def test_8_frames_per_game_second(self):
        """Exactly 8 render frames = 1 game-second."""
        gs = _make_gs()
        gs.game_seconds_counter = 0

        _run_frames(gs, 8)
        assert gs.game_seconds_counter == 1

    def test_480_frames_per_game_minute(self):
        """480 frames (8 frames × 60 seconds) = 1 game-minute."""
        gs = _make_gs()
        gs.time_minutes = 0
        gs.game_seconds_counter = 0

        _run_frames(gs, FRAMES_PER_MINUTE)
        assert gs.time_minutes == 1
        assert gs.game_seconds_counter == 0  # rolled over

    def test_28800_frames_per_game_hour(self):
        """28,800 frames = 1 game-hour."""
        gs = _make_gs()
        gs.time_hours = 8
        gs.time_minutes = 0
        gs.game_seconds_counter = 0

        _run_frames(gs, FRAMES_PER_HOUR)
        assert gs.time_hours == 9
        assert gs.time_minutes == 0

    def test_691200_frames_per_game_day(self):
        """691,200 frames = exactly 24 game-hours = 1 full day."""
        gs = _make_gs()
        gs.time_hours = 0
        gs.time_minutes = 0
        gs.game_seconds_counter = 0
        gs.date_day = 0

        _run_frames(gs, FRAMES_PER_DAY)
        assert gs.time_hours == 0
        assert gs.time_minutes == 0
        assert gs.date_day == 1  # calendar advanced by exactly 1 day

    def test_animation_tick_counter_increments_every_frame(self):
        """animation_tick_counter increments by exactly 1 per render frame."""
        gs = _make_gs()
        initial = gs.animation_tick_counter

        _run_frames(gs, 100)
        assert gs.animation_tick_counter == initial + 100

    def test_headless_no_rate_limit_by_default(self):
        """Headless mode without _realtime runs at full speed (no sleep)."""
        import time
        gs = _make_gs()
        assert not getattr(gs, '_realtime', False)

        start = time.monotonic()
        _run_frames(gs, 1000)
        elapsed = time.monotonic() - start

        # 1000 frames at 8 Hz would take 125 seconds; without rate
        # limiting it should complete in well under 1 second
        assert elapsed < 1.0, f"Headless mode too slow: {elapsed:.2f}s for 1000 frames"

    def test_realtime_rate_limits_to_8hz(self):
        """With _realtime=True, headless mode sleeps to maintain ~8 Hz."""
        import time
        gs = _make_gs()
        gs._realtime = True

        start = time.monotonic()
        _run_frames(gs, 4)  # 4 frames at 8 Hz = ~0.5 seconds
        elapsed = time.monotonic() - start

        gs._realtime = False  # clean up

        # Should take approximately 0.5 seconds (4 × 125ms)
        # Allow 0.3–0.8s range for timing jitter
        assert 0.3 < elapsed < 0.8, (
            f"Expected ~0.5s for 4 frames at 8 Hz, got {elapsed:.3f}s"
        )

    def test_frame_count_matches_game_seconds(self):
        """Verify exact ratio: N game-seconds require exactly N×8 frames."""
        gs = _make_gs()
        gs.game_seconds_counter = 0

        for expected_seconds in [1, 5, 10, 30, 60]:
            gs2 = _make_gs()
            gs2.game_seconds_counter = 0
            gs2.time_minutes = 0

            _run_frames(gs2, expected_seconds * FRAMES_PER_SECOND)

            actual_seconds = gs2.time_minutes * 60 + gs2.game_seconds_counter
            assert actual_seconds == expected_seconds, (
                f"Expected {expected_seconds} game-seconds after "
                f"{expected_seconds * FRAMES_PER_SECOND} frames, got {actual_seconds}"
            )

    def test_sub_animation_frame_counter_not_incremented_by_render(self):
        """sub_animation_frame_counter is incremented by game_tick_and_animate,
        NOT by screen_render_8hz_headless. Verify render alone doesn't touch it."""
        gs = _make_gs()
        initial = gs.sub_animation_frame_counter

        # Run render+simulate (our test helper), but sub_animation_frame_counter
        # is only incremented inside game_tick_and_animate, not by our helper
        screen_render_8hz_headless(gs)
        assert gs.sub_animation_frame_counter == initial

    def test_game_tick_and_animate_waits_for_counter(self):
        """game_tick_and_animate(gs, 0) waits for exactly 1 frame."""
        from lcp.main import game_tick_and_animate

        gs = _make_gs()
        initial_counter = gs.animation_tick_counter

        # game_tick_and_animate calls _screen_render_8hz which calls
        # screen_render_8hz_headless which increments animation_tick_counter
        game_tick_and_animate(gs, 0)

        # Counter should have advanced by at least 2 (one for the
        # busy-wait exit + one for the tail render call)
        assert gs.animation_tick_counter >= initial_counter + 2

    def test_game_tick_counter_n_waits_n_plus_1_frames(self):
        """game_tick_and_animate(gs, N) processes N+1 animation frames."""
        from lcp.main import game_tick_and_animate

        gs = _make_gs()
        initial_sub = gs.sub_animation_frame_counter

        game_tick_and_animate(gs, 4)

        # sub_animation_frame_counter incremented once per frame in the for loop
        assert gs.sub_animation_frame_counter == initial_sub + 5  # counter+1 = 5

    def test_clock_animation_4_frame_cycle(self):
        """Clock animation cycles through 4 frames (0,1,2,3) at 1/4 rate."""
        from lcp.main import game_tick_and_animate

        gs = _make_gs()
        frames_seen = set()

        for _ in range(16):
            game_tick_and_animate(gs, 0)
            frames_seen.add(gs.clock_animation_frame)

        # Should see all 4 clock frames (0, 1, 2, 3) within 16 ticks
        assert frames_seen == {0, 1, 2, 3}

    def test_minute_boundary_advances_clock(self):
        """At 60 game-seconds, time_minutes increments by 1."""
        gs = _make_gs()
        gs.time_minutes = 30
        gs.game_seconds_counter = 59  # one second before minute rollover

        _run_frames(gs, 8)  # 1 game-second
        assert gs.time_minutes == 31
        assert gs.game_seconds_counter == 0

    def test_hour_boundary_advances_clock(self):
        """At 60 game-minutes, time_hours increments by 1."""
        gs = _make_gs()
        gs.time_hours = 10
        gs.time_minutes = 59
        gs.game_seconds_counter = 59

        _run_frames(gs, 8)  # 1 game-second → rolls minute → rolls hour
        assert gs.time_hours == 11
        assert gs.time_minutes == 0

    def test_midnight_boundary_advances_calendar(self):
        """At hour 24 (midnight), date_day increments and daily flags reset."""
        gs = _make_gs()
        gs.time_hours = 23
        gs.time_minutes = 59
        gs.game_seconds_counter = 59
        gs.date_day = 5
        gs.lunch_meal_triggered_today = 1
        gs.dinner_meal_triggered_today = 1

        _run_frames(gs, 8)  # trigger midnight
        assert gs.time_hours == 0
        assert gs.date_day == 6
        assert gs.lunch_meal_triggered_today == 0
        assert gs.dinner_meal_triggered_today == 0

    def test_need_timers_decrement_per_minute(self):
        """Thirst/hunger/bathroom timers decrement once per game-minute."""
        gs = _make_gs()
        gs.lcp.thirst_timer = 50
        gs.lcp.hunger_timer = 80
        gs.lcp.bathroom_timer = 25
        gs.game_seconds_counter = 0

        _run_frames(gs, FRAMES_PER_MINUTE)  # 1 game-minute

        assert gs.lcp.thirst_timer == 49
        assert gs.lcp.hunger_timer == 79
        assert gs.lcp.bathroom_timer == 24

    def test_timing_constants_are_correct(self):
        """Verify our timing constants match the original hardware math."""
        # Original: 200 Hz timer, 25 ticks per frame = 8 Hz
        assert FRAMES_PER_SECOND == 8
        # 8 frames × 60 seconds = 480 frames per minute
        assert FRAMES_PER_MINUTE == 480
        # 480 × 60 = 28,800 frames per hour
        assert FRAMES_PER_HOUR == 28_800
        # 28,800 × 24 = 691,200 frames per day
        assert FRAMES_PER_DAY == 691_200
        # At 125ms per frame: 691,200 × 0.125 = 86,400 seconds = 24 hours
        assert FRAMES_PER_DAY * 0.125 == 86_400.0
